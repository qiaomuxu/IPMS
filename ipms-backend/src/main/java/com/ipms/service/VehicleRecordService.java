package com.ipms.service;

import com.ipms.entity.VehicleRecord;
import com.ipms.repository.VehicleRecordRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@Service
public class VehicleRecordService {

    @Autowired
    private VehicleRecordRepository vehicleRecordRepository;

    @Transactional
    public VehicleRecord recordVehicleEntry(String licensePlate, String plateColor, String imagePath, Integer confidence) {
        Optional<VehicleRecord> existingRecord = vehicleRecordRepository.findTopByLicensePlateAndStatusOrderByInTimeDesc(licensePlate, "in");

        if (existingRecord.isPresent()) {
            VehicleRecord record = existingRecord.get();
            record.setOutTime(LocalDateTime.now());
            record.setStatus("out");
            return vehicleRecordRepository.save(record);
        }

        VehicleRecord newRecord = new VehicleRecord();
        newRecord.setLicensePlate(licensePlate);
        newRecord.setPlateColor(plateColor);
        newRecord.setImagePath(imagePath);
        newRecord.setConfidence(confidence);
        newRecord.setInTime(LocalDateTime.now());
        newRecord.setStatus("in");

        return vehicleRecordRepository.save(newRecord);
    }

    public List<VehicleRecord> getAllRecords() {
        return vehicleRecordRepository.findAllByOrderByInTimeDesc();
    }

    public List<VehicleRecord> getRecordsByLicensePlate(String licensePlate) {
        return vehicleRecordRepository.findByLicensePlate(licensePlate);
    }

    public List<VehicleRecord> getCurrentInRecords() {
        return vehicleRecordRepository.findByStatusOrderByInTimeDesc("in");
    }

    public Optional<VehicleRecord> getRecordById(Long id) {
        return vehicleRecordRepository.findById(id);
    }

    @Transactional
    public Map<String, Object> processVehicleExit(String licensePlate) {
        Optional<VehicleRecord> existingRecord = vehicleRecordRepository.findTopByLicensePlateAndStatusOrderByInTimeDesc(licensePlate, "in");

        Map<String, Object> result = new HashMap<>();

        if (existingRecord.isPresent()) {
            VehicleRecord record = existingRecord.get();
            LocalDateTime outTime = LocalDateTime.now();
            record.setOutTime(outTime);
            record.setStatus("out");

            // 计算停车费用：每2秒1元
            long seconds = java.time.Duration.between(record.getInTime(), outTime).getSeconds();
            if (seconds < 0) seconds = 0;
            double fee = seconds / 2.0;
            record.setParkingFee(fee);

            VehicleRecord savedRecord = vehicleRecordRepository.save(record);

            result.put("success", true);
            result.put("licensePlate", savedRecord.getLicensePlate());
            result.put("inTime", savedRecord.getInTime());
            result.put("outTime", savedRecord.getOutTime());
            result.put("parkingFee", savedRecord.getParkingFee());
            result.put("message", "出场成功");
            return result;
        } else {
            result.put("success", false);
            result.put("licensePlate", licensePlate);
            result.put("message", "未找到该车牌的入场记录");
            return result;
        }
    }
}