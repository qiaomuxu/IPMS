package com.ipms.repository;

import com.ipms.entity.VehicleRecord;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;
import java.util.List;
import java.util.Optional;

@Repository
public interface VehicleRecordRepository extends JpaRepository<VehicleRecord, Long> {
    List<VehicleRecord> findByLicensePlate(String licensePlate);
    Optional<VehicleRecord> findTopByLicensePlateAndStatusOrderByInTimeDesc(String licensePlate, String status);
    List<VehicleRecord> findByStatusOrderByInTimeDesc(String status);
    List<VehicleRecord> findAllByOrderByInTimeDesc();
}