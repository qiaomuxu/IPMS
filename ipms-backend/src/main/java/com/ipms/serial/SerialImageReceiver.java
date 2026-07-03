package com.ipms.serial;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;

import com.ipms.cloud.TencentCloudOcrService;
import com.ipms.entity.VehicleRecord;
import com.ipms.repository.VehicleRecordRepository;
import com.ipms.service.VehicleRecordService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Date;
import java.util.Map;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

@Component
public class SerialImageReceiver {

    private static final Logger logger = LoggerFactory.getLogger(SerialImageReceiver.class);

    private static final int IMAGE_WIDTH = 320;
    private static final int IMAGE_HEIGHT = 240;
    private static final int IMAGE_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * 2;

    @Value("${serial.port:COM7}")
    private String portName;

    @Value("${serial.baudrate:115200}")
    private int baudRate;

    @Value("${upload.path:./uploads}")
    private String uploadPath;

    @Autowired(required = false)
    private TencentCloudOcrService ocrService;

    @Autowired
    private VehicleRecordService vehicleRecordService;

    @Autowired
    private VehicleRecordRepository vehicleRecordRepository;

    private SerialPort serialPort;
    private InputStream inputStream;
    private OutputStream outputStream;
    private volatile boolean listenerRunning = true;

    private ByteArrayOutputStream imageBuffer;
    private int bytesReceived = 0;

    @PostConstruct
    public void init() {
        connect();
    }

    @PreDestroy
    public void disconnect() {
        listenerRunning = false;
        if (serialPort != null && serialPort.isOpen()) {
            serialPort.closePort();
            logger.info("Serial port closed");
        }
    }

    public void connect() {
        try {
            serialPort = SerialPort.getCommPort(portName);
            serialPort.setBaudRate(baudRate);
            serialPort.setNumDataBits(8);
            serialPort.setNumStopBits(1);
            serialPort.setParity(0);
            serialPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);

            if (serialPort.openPort()) {
                inputStream = serialPort.getInputStream();
                outputStream = serialPort.getOutputStream();
                serialPort.addDataListener(new SerialPortReader());
                logger.info("Connected to serial port {} at {} baud", portName, baudRate);
            } else {
                logger.error("Failed to open serial port {}", portName);
            }
        } catch (Exception e) {
            logger.error("Failed to connect to serial port: {}", e.getMessage());
        }
    }

    public boolean isConnected() {
        return serialPort != null && serialPort.isOpen();
    }

    public String getPortName() {
        return portName;
    }

    public int getBaudRate() {
        return baudRate;
    }

    private class SerialPortReader implements SerialPortDataListener {
        @Override
        public int getListeningEvents() {
            return SerialPort.LISTENING_EVENT_DATA_AVAILABLE;
        }

        @Override
        public void serialEvent(SerialPortEvent event) {
            if (event.getEventType() == SerialPort.LISTENING_EVENT_DATA_AVAILABLE) {
                try {
                    int available = inputStream.available();
                    if (available > 0) {
                        byte[] buffer = new byte[available];
                        int bytesRead = inputStream.read(buffer);
                        if (bytesRead > 0) {
                            logger.debug("Received {} bytes, current buffer size: {}", bytesRead, bytesReceived);
                            for (byte b : buffer) {
                                if (imageBuffer == null) {
                                    imageBuffer = new ByteArrayOutputStream();
                                    bytesReceived = 0;
                                    logger.info("Starting new image capture");
                                }
                                imageBuffer.write(b);
                                bytesReceived++;
                                if (bytesReceived % 10000 == 0) {
                                    logger.info("Received {} / {} bytes", bytesReceived, IMAGE_SIZE);
                                }
                                if (bytesReceived >= IMAGE_SIZE) {
                                    byte[] imageData = imageBuffer.toByteArray();
                                    saveImage(imageData);
                                    imageBuffer = null;
                                    bytesReceived = 0;
                                }
                            }
                        }
                    }
                } catch (IOException e) {
                    logger.error("Error reading from serial port", e);
                }
            }
        }
    }

    private void saveImage(byte[] rgb565Data) {
        logger.info("saveImage called with {} bytes", rgb565Data.length);
        try {
            Path uploadDir = Paths.get(uploadPath);
            if (!Files.exists(uploadDir)) {
                Files.createDirectories(uploadDir);
            }

            String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss_SSS").format(new Date());
            String fileName = "image_" + timestamp;
            Path rgb565Path = uploadDir.resolve(fileName + ".rgb565");
            Path pngPath = uploadDir.resolve(fileName + ".png");

            try (FileOutputStream fos = new FileOutputStream(rgb565Path.toFile())) {
                fos.write(rgb565Data);
            }

            logger.info("RGB565 image saved to: {}", rgb565Path);
            convertAndSaveAsPng(rgb565Data, pngPath.toFile());
            performOcrRecognition(pngPath.toFile());

        } catch (IOException e) {
            logger.error("Error saving image", e);
        }
    }

    private void convertAndSaveAsPng(byte[] rgb565Data, File pngFile) {
        try {
            int w = IMAGE_WIDTH;
            int h = IMAGE_HEIGHT;
            java.awt.image.BufferedImage image = new java.awt.image.BufferedImage(w, h, java.awt.image.BufferedImage.TYPE_INT_ARGB);

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int index = (y * w + x) * 2;
                    if (index + 1 < rgb565Data.length) {
                        int byte0 = rgb565Data[index] & 0xFF;
                        int byte1 = rgb565Data[index + 1] & 0xFF;
                        int r = byte1 & 0xF8;
                        int g = ((byte0 << 1) | ((byte1 & 0x80) >> 7)) & 0xFC;
                        int b = byte0 & 0xF8;
                        int argb = (0xFF << 24) | (r << 16) | (g << 8) | b;
                        image.setRGB(x, y, argb);
                    }
                }
            }

            javax.imageio.ImageIO.write(image, "PNG", pngFile);
            logger.info("PNG saved to: {}", pngFile.getAbsolutePath());
        } catch (IOException e) {
            logger.error("Error converting to PNG", e);
        }
    }

    private void performOcrRecognition(File pngFile) {
        if (ocrService == null || !ocrService.isAvailable()) {
            logger.debug("OCR service not available, skipping recognition");
            return;
        }

        try {
            String recognizedText = ocrService.recognizeText(pngFile);
            if (recognizedText != null && !recognizedText.isEmpty()) {
                logger.info("OCR Result: {}", recognizedText);

                String licensePlate = extractLicensePlate(recognizedText);
                String plateColor = extractPlateColor(recognizedText);
                Integer confidence = extractConfidence(recognizedText);

                if (licensePlate != null && !licensePlate.isEmpty()) {
                    // 先检查该车牌是否有入场记录
                    Optional<VehicleRecord> existingRecord = vehicleRecordRepository
                            .findTopByLicensePlateAndStatusOrderByInTimeDesc(licensePlate, "in");

                    if (existingRecord.isPresent()) {
                        // 该车牌已在库中（入场状态），触发出库
                        logger.info("Vehicle {} already in parking, processing exit", licensePlate);
                        Map<String, Object> exitResult = vehicleRecordService.processVehicleExit(licensePlate);

                        if ((boolean) exitResult.get("success")) {
                            double parkingFee = (double) exitResult.get("parkingFee");
                            LocalDateTime inTime = (LocalDateTime) exitResult.get("inTime");
                            LocalDateTime outTime = (LocalDateTime) exitResult.get("outTime");

                            logger.info("Vehicle exit processed: {}, fee: {}", licensePlate, parkingFee);

                            // 发送出库结果到STM32
                            sendExitResultToHardware(licensePlate, parkingFee, inTime, outTime);
                        }
                    } else {
                        // 没有入场记录，创建新入场记录
                        VehicleRecord record = vehicleRecordService.recordVehicleEntry(
                                licensePlate,
                                plateColor,
                                pngFile.getAbsolutePath(),
                                confidence
                        );
                        logger.info("Vehicle record saved: {}", licensePlate);

                        sendResultToHardware(
                                licensePlate,
                                plateColor,
                                confidence,
                                record.getInTime(),
                                record.getOutTime(),
                                record.getStatus()
                        );
                    }
                }
            } else {
                logger.info("No text detected in image");
            }
        } catch (Exception e) {
            logger.error("OCR recognition failed: {}", e.getMessage());
        }
    }

    private void sendExitResultToHardware(String licensePlate, double fee, LocalDateTime inTime, LocalDateTime outTime) {
        try {
            if (outputStream != null && serialPort != null && serialPort.isOpen()) {
                DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

                String plateWithPinyin = convertProvinceToPinyin(licensePlate);

                String result = String.format("EXIT_OK:%s,FEE:%.2f,IN:%s,OUT:%s\r\n",
                        plateWithPinyin,
                        fee,
                        inTime != null ? inTime.format(formatter) : "--:--:--",
                        outTime != null ? outTime.format(formatter) : "--:--:--");

                outputStream.write(result.getBytes(StandardCharsets.UTF_8));
                outputStream.flush();

                logger.info("Exit result sent to hardware: {}", result.trim());
            }
        } catch (IOException e) {
            logger.error("Failed to send exit result to hardware: {}", e.getMessage());
        }
    }

    private void sendResultToHardware(String licensePlate, String plateColor, Integer confidence, LocalDateTime inTime, LocalDateTime outTime, String status) {
        try {
            if (outputStream != null && serialPort != null && serialPort.isOpen()) {
                DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

                // 计算停车费用：每2秒1元
                long fee = 0;
                if (inTime != null) {
                    long seconds = java.time.Duration.between(inTime, LocalDateTime.now()).getSeconds();
                    if (seconds < 0) seconds = 0;
                    fee = seconds / 2;
                }

                String colorEng = convertColorToEnglish(plateColor);
                String plateWithPinyin = convertProvinceToPinyin(licensePlate);

                String result = String.format("PLATE:%s,COLOR:%s,IN:%s,OUT:%s,STATUS:%s,FEE:%d\r\n",
                        plateWithPinyin,
                        colorEng,
                        inTime != null ? inTime.format(formatter) : "--:--:--",
                        outTime != null ? outTime.format(formatter) : "--:--:--",
                        status != null ? status : "in",
                        fee);

                outputStream.write(result.getBytes(StandardCharsets.UTF_8));
                outputStream.flush();

                logger.info("Result sent to hardware: {}", result.trim());
            } else {
                logger.warn("Cannot send result to hardware: serial port not available");
            }
        } catch (IOException e) {
            logger.error("Failed to send result to hardware: {}", e.getMessage());
        }
    }

    private String convertColorToEnglish(String chineseColor) {
        if (chineseColor == null) return "Unknown";
        switch (chineseColor) {
            case "蓝": return "Blue";
            case "黄": return "Yellow";
            case "绿": return "Green";
            case "黑": return "Black";
            case "白": return "White";
            default: return chineseColor;
        }
    }

    private String convertProvinceToPinyin(String licensePlate) {
        if (licensePlate == null || licensePlate.isEmpty()) return licensePlate;
        String firstChar = licensePlate.substring(0, 1);
        String pinyin = provinceToPinyin(firstChar);
        if (pinyin != null) {
            return pinyin + licensePlate.substring(1);
        }
        return licensePlate;
    }

    private String provinceToPinyin(String province) {
        switch (province) {
            case "京": return "J";
            case "津": return "T";
            case "沪": return "H";
            case "渝": return "Y";
            case "冀": return "HB";
            case "豫": return "Y";
            case "云": return "YN";
            case "辽": return "LN";
            case "黑": return "HLJ";
            case "湘": return "HN";
            case "皖": return "AH";
            case "鲁": return "SD";
            case "新": return "XJ";
            case "苏": return "JS";
            case "浙": return "ZJ";
            case "赣": return "JX";
            case "鄂": return "HB";
            case "粤": return "GD";
            case "桂": return "GX";
            case "琼": return "HN";
            case "川": return "SC";
            case "贵": return "GZ";
            case "藏": return "XZ";
            case "陕": return "SN";
            case "甘": return "GS";
            case "青": return "QH";
            case "宁": return "NX";
            case "蒙": return "NM";
            case "晋": return "SX";
            case "警": return "JJ";
            default: return province;
        }
    }

    private String extractLicensePlate(String ocrResult) {
        Pattern pattern = Pattern.compile("车牌号码:\\s*([\\u4e00-\\u9fa5A-Z0-9]+)");
        Matcher matcher = pattern.matcher(ocrResult);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return null;
    }

    private String extractPlateColor(String ocrResult) {
        Pattern pattern = Pattern.compile("车牌颜色:\\s*(\\S+)");
        Matcher matcher = pattern.matcher(ocrResult);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return null;
    }

    private Integer extractConfidence(String ocrResult) {
        Pattern pattern = Pattern.compile("置信度:\\s*(\\d+)");
        Matcher matcher = pattern.matcher(ocrResult);
        if (matcher.find()) {
            return Integer.parseInt(matcher.group(1));
        }
        return null;
    }

    /**
     * 发送舵机控制命令
     * @param command OPEN-抬杆，CLOSE-落杆
     */
    public void sendGateCommand(String command) {
        try {
            if (outputStream != null && serialPort != null && serialPort.isOpen()) {
                String cmd = String.format("GATE:%s\r\n", command);
                outputStream.write(cmd.getBytes(StandardCharsets.UTF_8));
                outputStream.flush();
                logger.info("Gate command sent: {}", command);
            } else {
                logger.warn("Cannot send gate command: serial port not available");
            }
        } catch (IOException e) {
            logger.error("Failed to send gate command: {}", e.getMessage());
            throw new RuntimeException("发送舵机命令失败", e);
        }
    }
}
