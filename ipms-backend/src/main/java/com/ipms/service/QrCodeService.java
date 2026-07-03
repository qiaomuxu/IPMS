package com.ipms.service;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.WriterException;
import com.google.zxing.client.j2se.MatrixToImageWriter;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

@Service
public class QrCodeService {

    @Value("${server.address:localhost}")
    private String serverAddress;

    @Value("${server.port:8080}")
    private int serverPort;

    @Value("${upload.path:./uploads}")
    private String uploadPath;

    private static final int QR_CODE_SIZE = 250;

    /**
     * 生成支付二维码
     * @param orderId 订单ID
     * @return 二维码图片路径
     */
    public String generatePaymentQRCode(Long orderId) {
        String payUrl = String.format("http://%s:%d/pay.html?orderId=%d", serverAddress, serverPort, orderId);
        return generateQRCodeImage(payUrl, orderId + ".png");
    }

    /**
     * 生成二维码图片
     * @param content 二维码内容
     * @param fileName 文件名
     * @return 文件路径
     */
    public String generateQRCodeImage(String content, String fileName) {
        try {
            // 确保目录存在
            Path uploadDir = Paths.get(uploadPath);
            if (!Files.exists(uploadDir)) {
                Files.createDirectories(uploadDir);
            }

            // 生成二维码
            QRCodeWriter writer = new QRCodeWriter();
            Map<EncodeHintType, Object> hints = new HashMap<>();
            hints.put(EncodeHintType.CHARACTER_SET, "UTF-8");
            hints.put(EncodeHintType.MARGIN, 1);

            BitMatrix bitMatrix = writer.encode(content, BarcodeFormat.QR_CODE, QR_CODE_SIZE, QR_CODE_SIZE, hints);

            // 转换为图片
            BufferedImage image = MatrixToImageWriter.toBufferedImage(bitMatrix);

            // 保存为PNG
            Path filePath = uploadDir.resolve(fileName);
            File outputFile = filePath.toFile();
            ImageIO.write(image, "PNG", outputFile);

            return filePath.toString();
        } catch (WriterException | IOException e) {
            throw new RuntimeException("生成二维码失败: " + e.getMessage(), e);
        }
    }

    /**
     * 生成二维码并返回Base64编码
     * @param content 二维码内容
     * @return Base64编码的图片数据
     */
    public String generateQRCodeBase64(String content) {
        try {
            QRCodeWriter writer = new QRCodeWriter();
            Map<EncodeHintType, Object> hints = new HashMap<>();
            hints.put(EncodeHintType.CHARACTER_SET, "UTF-8");
            hints.put(EncodeHintType.MARGIN, 1);

            BitMatrix bitMatrix = writer.encode(content, BarcodeFormat.QR_CODE, QR_CODE_SIZE, QR_CODE_SIZE, hints);
            BufferedImage image = MatrixToImageWriter.toBufferedImage(bitMatrix);

            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            ImageIO.write(image, "PNG", baos);
            byte[] imageBytes = baos.toByteArray();

            return "data:image/png;base64," + java.util.Base64.getEncoder().encodeToString(imageBytes);
        } catch (WriterException | IOException e) {
            throw new RuntimeException("生成二维码失败: " + e.getMessage(), e);
        }
    }
}