package com.kc8qzo.arduinocommunication.service;

import com.kc8qzo.arduinocommunication.contracts.dto.SensorReadingDTO;
import com.kc8qzo.arduinocommunication.db.domain.SensorReading;
import com.kc8qzo.arduinocommunication.db.repositories.ArduinoSensorRepository;
import lombok.RequiredArgsConstructor;
import org.modelmapper.ModelMapper;
import org.springframework.data.domain.Sort;
import org.springframework.stereotype.Service;

import java.time.Clock;
import java.time.Instant;
import java.util.List;
import java.util.Optional;

@Service
@RequiredArgsConstructor
public class SensorService {
    private final ModelMapper mapper;
    private final ArduinoSensorRepository arduinoSensorRepository;
    private final Clock clock;

    public void receiveSensorData(SensorReadingDTO dto) {
        SensorReading entity = mapper.map(dto, SensorReading.class);
        if (entity.getPostedAt() == null) {
            entity.setPostedAt(Instant.now(clock));
        }
        arduinoSensorRepository.save(entity);
    }

    public List<SensorReadingDTO> retrieveSensorReadings() {
        List<SensorReading> sensorsReadings = arduinoSensorRepository.findAll(Sort.by(Sort.Direction.DESC, "id"));
        return sensorsReadings.stream().map(sensorsReading -> mapper.map(sensorsReading, SensorReadingDTO.class)).toList();
    }

    public Optional<SensorReadingDTO> getLatestSensorReading() {
        return arduinoSensorRepository.findFirstByOrderByIdDesc()
                .map(this::mapToSensorReadingDTO);
    }

    private SensorReadingDTO mapToSensorReadingDTO(SensorReading sensorReading) {
        return mapper.map(sensorReading, SensorReadingDTO.class);
    }
}
