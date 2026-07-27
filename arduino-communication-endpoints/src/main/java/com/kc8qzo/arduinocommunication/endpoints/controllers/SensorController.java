package com.kc8qzo.arduinocommunication.endpoints.controllers;

import com.kc8qzo.arduinocommunication.contracts.dto.SensorReadingDTO;
import com.kc8qzo.arduinocommunication.service.SensorService;
import io.swagger.annotations.ApiOperation;
import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RequiredArgsConstructor
@RestController
@RequestMapping("/api/sensors")
public class SensorController {
    private final SensorService sensorService;

    @PostMapping
    public String receive(@RequestBody SensorReadingDTO readingDTO) {
        sensorService.receiveSensorData(readingDTO);
        System.out.println("Temp = " + readingDTO.getTemperature());
        System.out.println("Humidity = " + readingDTO.getHumidity());
        System.out.println("Light = " + readingDTO.getLight());
        System.out.println("Pass Value = " + readingDTO.getPassValue());
        System.out.println("SentAt = " + readingDTO.getSentAt());
        System.out.println("TimeStamp = " + readingDTO.getPostedAt());
        return "OK";
    }

    @ApiOperation(value = "Retrieves SensorReadings", response = SensorReadingDTO.class, httpMethod = "GET")
    @ResponseBody
    @CrossOrigin
    @GetMapping("/readings")
    public List<SensorReadingDTO> retrieveSensorReadings() {

        return sensorService.retrieveSensorReadings();
    }

    @ApiOperation(value = "Retrieves Latest SensorReading", response = SensorReadingDTO.class, httpMethod = "GET")
    @ResponseBody
    @CrossOrigin
    @GetMapping("/latest")
    public ResponseEntity<SensorReadingDTO> retrieveLatestSensorReading() {
        return sensorService.getLatestSensorReading()
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.noContent().build());
    }

    @ApiOperation(value = "Updates SensorReading", response = SensorReadingDTO.class, httpMethod = "PUT")
    @ResponseBody
    @CrossOrigin
    @PutMapping("/readings/{id}")
    public ResponseEntity<SensorReadingDTO> updateSensorReading(@PathVariable Long id,
                                                                @RequestBody SensorReadingDTO reading) {
        return sensorService.updateSensorReading(id, reading)
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }
}
