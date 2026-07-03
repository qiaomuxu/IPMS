package com.ipms.cloud;

import com.tencentcloudapi.common.Credential;
import com.tencentcloudapi.common.profile.ClientProfile;
import com.tencentcloudapi.common.profile.HttpProfile;
import com.tencentcloudapi.common.exception.TencentCloudSDKException;
import com.tencentcloudapi.ocr.v20181119.OcrClient;
import com.tencentcloudapi.ocr.v20181119.models.LicensePlateOCRRequest;
import com.tencentcloudapi.ocr.v20181119.models.LicensePlateOCRResponse;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.File;
import java.nio.file.Files;

@Service
public class TencentCloudOcrService {

    private static final Logger logger = LoggerFactory.getLogger(TencentCloudOcrService.class);

    @Value("${tencent.cloud.secret-id:}")
    private String secretId;

    @Value("${tencent.cloud.secret-key:}")
    private String secretKey;

    @Value("${tencent.cloud.region:ap-guangzhou}")
    private String region;

    public String recognizeText(File imageFile) {
        if (!isAvailable()) {
            logger.error("Tencent Cloud credentials not configured. Please set tencent.cloud.secret-id and tencent.cloud.secret-key");
            return null;
        }

        try {
            Credential credential = new Credential(secretId, secretKey);

            HttpProfile httpProfile = new HttpProfile();
            httpProfile.setEndpoint("ocr.tencentcloudapi.com");

            ClientProfile clientProfile = new ClientProfile();
            clientProfile.setHttpProfile(httpProfile);

            OcrClient client = new OcrClient(credential, region, clientProfile);

            String imageBase64 = java.util.Base64.getEncoder().encodeToString(Files.readAllBytes(imageFile.toPath()));

            LicensePlateOCRRequest req = new LicensePlateOCRRequest();
            req.setImageBase64(imageBase64);

            LicensePlateOCRResponse resp = client.LicensePlateOCR(req);

            StringBuilder result = new StringBuilder();
            result.append("车牌号码: ").append(resp.getNumber());
            result.append("\n车牌颜色: ").append(resp.getColor());
            result.append("\n置信度: ").append(resp.getConfidence()).append("%");

            logger.info("License plate recognized: {}, color: {}, confidence: {}%", resp.getNumber(), resp.getColor(), resp.getConfidence());
            return result.toString();

        } catch (TencentCloudSDKException e) {
            logger.error("OCR API Error: {}", e.getMessage());
            return null;
        } catch (Exception e) {
            logger.error("OCR recognition failed: {}", e.getMessage());
            return null;
        }
    }

    public boolean isAvailable() {
        return secretId != null && !secretId.isEmpty()
                && secretKey != null && !secretKey.isEmpty();
    }
}