package com.ipms.controller;

import com.ipms.serial.SerialImageReceiver;
import com.ipms.service.QrCodeService;
import com.ipms.service.VehicleRecordService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.Map;

@RestController
@RequestMapping("/api/payment")
@CrossOrigin(origins = "*")
public class PaymentController {

    private static final Logger logger = LoggerFactory.getLogger(PaymentController.class);

    @Autowired
    private QrCodeService qrCodeService;

    @Autowired
    private VehicleRecordService vehicleRecordService;

    @Autowired(required = false)
    private SerialImageReceiver serialImageReceiver;

    /**
     * 生成支付二维码
     * GET /api/payment/qrcode/{orderId}
     */
    @GetMapping("/qrcode/{orderId}")
    public ResponseEntity<Map<String, Object>> generatePaymentQRCode(@PathVariable Long orderId) {
        Map<String, Object> response = new HashMap<>();

        try {
            // 生成二维码图片，返回文件路径
            String qrCodePath = qrCodeService.generatePaymentQRCode(orderId);

            // 返回完整的访问URL
            String qrCodeUrl = qrCodePath.replace("\\", "/");
            if (qrCodeUrl.contains("/uploads/")) {
                qrCodeUrl = "/uploads/" + qrCodeUrl.substring(qrCodeUrl.lastIndexOf("/uploads/") + 9);
            }

            // 获取订单信息
            var record = vehicleRecordService.getRecordByIdValue(orderId);
            if (record == null) {
                response.put("success", false);
                response.put("message", "订单不存在");
                return ResponseEntity.ok(response);
            }

            response.put("success", true);
            response.put("qrCodeUrl", qrCodeUrl);
            response.put("orderId", orderId);
            response.put("amount", record.getParkingFee());
            response.put("plateNumber", record.getLicensePlate());

            logger.info("生成支付二维码成功: orderId={}, qrCodeUrl={}", orderId, qrCodeUrl);

            return ResponseEntity.ok(response);
        } catch (Exception e) {
            logger.error("生成支付二维码失败: orderId={}", orderId, e);
            response.put("success", false);
            response.put("message", "生成二维码失败: " + e.getMessage());
            return ResponseEntity.internalServerError().body(response);
        }
    }

    /**
     * 确认支付
     * POST /api/payment/confirm
     */
    @PostMapping("/confirm")
    public ResponseEntity<Map<String, Object>> confirmPayment(@RequestBody Map<String, Object> request) {
        Map<String, Object> response = new HashMap<>();

        try {
            Long orderId = Long.parseLong(request.get("orderId").toString());

            // 更新订单状态为已支付
            boolean updated = vehicleRecordService.updatePaymentStatus(orderId, "paid");

            if (!updated) {
                response.put("success", false);
                response.put("message", "更新订单状态失败");
                return ResponseEntity.ok(response);
            }

            // 发送支付成功命令到STM32
            if (serialImageReceiver != null && serialImageReceiver.isConnected()) {
                serialImageReceiver.sendPaymentSuccess(orderId);
                logger.info("支付成功，已发送通知到STM32: orderId={}", orderId);
            } else {
                logger.warn("STM32未连接，无法发送支付成功通知: orderId={}", orderId);
            }

            response.put("success", true);
            response.put("message", "支付成功");

            return ResponseEntity.ok(response);
        } catch (Exception e) {
            logger.error("确认支付失败", e);
            response.put("success", false);
            response.put("message", "支付确认失败: " + e.getMessage());
            return ResponseEntity.internalServerError().body(response);
        }
    }

    /**
     * 查询支付状态
     * GET /api/payment/status/{orderId}
     */
    @GetMapping("/status/{orderId}")
    public ResponseEntity<Map<String, Object>> getPaymentStatus(@PathVariable Long orderId) {
        Map<String, Object> response = new HashMap<>();

        try {
            var record = vehicleRecordService.getRecordByIdValue(orderId);
            if (record == null) {
                response.put("success", false);
                response.put("message", "订单不存在");
                return ResponseEntity.ok(response);
            }

            response.put("success", true);
            response.put("orderId", orderId);
            response.put("status", record.getStatus());
            response.put("paid", "paid".equals(record.getStatus()));

            return ResponseEntity.ok(response);
        } catch (Exception e) {
            logger.error("查询支付状态失败: orderId={}", orderId, e);
            response.put("success", false);
            response.put("message", e.getMessage());
            return ResponseEntity.internalServerError().body(response);
        }
    }
}