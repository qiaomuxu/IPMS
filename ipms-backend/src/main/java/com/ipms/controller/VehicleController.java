package com.ipms.controller;

import com.ipms.entity.VehicleRecord;
import com.ipms.service.VehicleRecordService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/vehicle")
@CrossOrigin(origins = "*")
public class VehicleController {

    @Autowired
    private VehicleRecordService vehicleRecordService;

    @GetMapping("/records")
    public ResponseEntity<List<VehicleRecord>> getAllRecords() {
        return ResponseEntity.ok(vehicleRecordService.getAllRecords());
    }

    @GetMapping("/records/in")
    public ResponseEntity<List<VehicleRecord>> getCurrentInRecords() {
        return ResponseEntity.ok(vehicleRecordService.getCurrentInRecords());
    }

    @GetMapping("/records/{id}")
    public ResponseEntity<VehicleRecord> getRecordById(@PathVariable Long id) {
        return vehicleRecordService.getRecordById(id)
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }

    @GetMapping("/records/search")
    public ResponseEntity<List<VehicleRecord>> searchByLicensePlate(@RequestParam String licensePlate) {
        return ResponseEntity.ok(vehicleRecordService.getRecordsByLicensePlate(licensePlate));
    }

    @PostMapping("/record")
    public ResponseEntity<Map<String, Object>> recordVehicle(
            @RequestParam String licensePlate,
            @RequestParam(required = false) String plateColor,
            @RequestParam(required = false) String imagePath,
            @RequestParam(required = false) Integer confidence) {

        VehicleRecord record = vehicleRecordService.recordVehicleEntry(
                licensePlate, plateColor, imagePath, confidence);

        Map<String, Object> response = new HashMap<>();
        response.put("id", record.getId());
        response.put("licensePlate", record.getLicensePlate());
        response.put("status", record.getStatus());
        response.put("inTime", record.getInTime());
        response.put("outTime", record.getOutTime());

        return ResponseEntity.ok(response);
    }

    @GetMapping("/stats")
    public ResponseEntity<Map<String, Object>> getStats() {
        List<VehicleRecord> allRecords = vehicleRecordService.getAllRecords();
        List<VehicleRecord> inRecords = vehicleRecordService.getCurrentInRecords();

        Map<String, Object> stats = new HashMap<>();
        stats.put("totalRecords", allRecords.size());
        stats.put("currentIn", inRecords.size());
        stats.put("totalOut", allRecords.size() - inRecords.size());

        return ResponseEntity.ok(stats);
    }

    @PostMapping("/exit")
    public ResponseEntity<Map<String, Object>> vehicleExit(@RequestParam String licensePlate) {
        Map<String, Object> result = vehicleRecordService.processVehicleExit(licensePlate);
        return ResponseEntity.ok(result);
    }
}