package com.kc8qzo.arduinocommunication.db.repositories;

import com.kc8qzo.arduinocommunication.db.domain.SensorReading;
import org.springframework.data.jpa.repository.JpaRepository;
import java.util.Optional;


public interface ArduinoSensorRepository extends JpaRepository<SensorReading,Long> {
    Optional<SensorReading> findFirstByOrderByIdDesc();
}
