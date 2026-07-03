package com.ipms.controller;

import com.ipms.serial.SerialImageReceiver;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.util.*;

@RestController
@RequestMapping("/api/images")
@CrossOrigin(origins = "*")
public class ImageController {

    private static final Logger logger = LoggerFactory.getLogger(ImageController.class);

    @Autowired
    private SerialImageReceiver imageReceiver;

    @Value("${upload.path:./uploads}")
    private String uploadPath;

    private final List<Map<String, Object>> imageHistory = Collections.synchronizedList(new ArrayList<>());

    @GetMapping("/status")
    public Map<String, Object> getStatus() {
        Map<String, Object> status = new HashMap<>();
        status.put("connected", imageReceiver.isConnected());
        status.put("port", imageReceiver.getPortName());
        status.put("baudrate", imageReceiver.getBaudRate());
        status.put("totalImages", imageHistory.size());
        return status;
    }

    @GetMapping("/list")
    public List<Map<String, Object>> listImages() {
        return new ArrayList<>(imageHistory);
    }

    @GetMapping("/latest")
    public ResponseEntity<Resource> getLatestImage() {
        if (imageHistory.isEmpty()) {
            return ResponseEntity.notFound().build();
        }

        Map<String, Object> latest = imageHistory.get(imageHistory.size() - 1);
        String filePath = (String) latest.get("pngPath");

        File file = new File(filePath);
        if (!file.exists()) {
            return ResponseEntity.notFound().build();
        }

        Resource resource = new FileSystemResource(file);
        return ResponseEntity.ok()
                .header(HttpHeaders.CONTENT_DISPOSITION, "inline")
                .contentType(MediaType.IMAGE_PNG)
                .body(resource);
    }

    @GetMapping("/{timestamp}")
    public ResponseEntity<Resource> getImage(@PathVariable String timestamp) {
        for (Map<String, Object> img : imageHistory) {
            if (img.get("timestamp").equals(timestamp)) {
                String filePath = (String) img.get("pngPath");
                File file = new File(filePath);
                if (file.exists()) {
                    Resource resource = new FileSystemResource(file);
                    return ResponseEntity.ok()
                            .header(HttpHeaders.CONTENT_DISPOSITION, "inline")
                            .contentType(MediaType.IMAGE_PNG)
                            .body(resource);
                }
            }
        }
        return ResponseEntity.notFound().build();
    }

    @GetMapping("/{timestamp}/raw")
    public ResponseEntity<Resource> getRawImage(@PathVariable String timestamp) {
        for (Map<String, Object> img : imageHistory) {
            if (img.get("timestamp").equals(timestamp)) {
                String filePath = (String) img.get("rgb565Path");
                File file = new File(filePath);
                if (file.exists()) {
                    Resource resource = new FileSystemResource(file);
                    return ResponseEntity.ok()
                            .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=image.rgb565")
                            .contentType(MediaType.APPLICATION_OCTET_STREAM)
                            .body(resource);
                }
            }
        }
        return ResponseEntity.notFound().build();
    }

    @PostMapping("/upload")
    public ResponseEntity<Map<String, Object>> uploadImage(@RequestParam("file") MultipartFile file) {
        Map<String, Object> response = new HashMap<>();
        try {
            Path uploadDir = Paths.get(uploadPath);
            if (!Files.exists(uploadDir)) {
                Files.createDirectories(uploadDir);
            }

            String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss_SSS").format(new Date());
            String fileName = "upload_" + timestamp + ".png";
            Path filePath = uploadDir.resolve(fileName);

            Files.copy(file.getInputStream(), filePath);

            Map<String, Object> imageInfo = new HashMap<>();
            imageInfo.put("timestamp", timestamp);
            imageInfo.put("pngPath", filePath.toString());
            imageInfo.put("rgb565Path", null);
            imageInfo.put("width", 0);
            imageInfo.put("height", 0);
            imageInfo.put("size", file.getSize());
            imageInfo.put("type", "upload");

            imageHistory.add(imageInfo);

            response.put("success", true);
            response.put("timestamp", timestamp);
            response.put("path", filePath.toString());

            return ResponseEntity.ok(response);
        } catch (IOException e) {
            logger.error("Error uploading image", e);
            response.put("success", false);
            response.put("error", e.getMessage());
            return ResponseEntity.internalServerError().body(response);
        }
    }

    @DeleteMapping("/clear")
    public ResponseEntity<Map<String, String>> clearHistory() {
        imageHistory.clear();
        Map<String, String> response = new HashMap<>();
        response.put("status", "cleared");
        return ResponseEntity.ok(response);
    }

    @GetMapping("/count")
    public Map<String, Long> getImageCount() {
        Map<String, Long> count = new HashMap<>();
        count.put("total", (long) imageHistory.size());
        return count;
    }

    public void addToHistory(String timestamp, String rgb565Path, String pngPath, int width, int height) {
        Map<String, Object> imageInfo = new HashMap<>();
        imageInfo.put("timestamp", timestamp);
        imageInfo.put("rgb565Path", rgb565Path);
        imageInfo.put("pngPath", pngPath);
        imageInfo.put("width", width);
        imageInfo.put("height", height);
        imageInfo.put("type", "serial");
        imageInfo.put("size", new File(rgb565Path).length());
        imageHistory.add(imageInfo);
    }

    @PostMapping("/gate/open")
    public Map<String, Object> openGate() {
        Map<String, Object> response = new HashMap<>();
        try {
            imageReceiver.sendGateCommand("OPEN");
            response.put("success", true);
            response.put("message", "抬杆命令已发送");
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "发送失败: " + e.getMessage());
        }
        return response;
    }

    @PostMapping("/gate/close")
    public Map<String, Object> closeGate() {
        Map<String, Object> response = new HashMap<>();
        try {
            imageReceiver.sendGateCommand("CLOSE");
            response.put("success", true);
            response.put("message", "落杆命令已发送");
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "发送失败: " + e.getMessage());
        }
        return response;
    }
}
