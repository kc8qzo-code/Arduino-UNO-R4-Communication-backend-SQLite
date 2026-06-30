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
    public String receive(@RequestBody SensorReadingDTO reading) {
        sensorService.receiveSensorData(reading);
        System.out.println("Temp = " + reading.getTemperature());
        System.out.println("Humidity = " + reading.getHumidity());
        System.out.println("Light = " + reading.getLight());
        return "OK";
    }

    @ApiOperation(value = "Retrieves SensorReadings", response = SensorReadingDTO.class, httpMethod = "GET")
    @ResponseBody
    @GetMapping("/readings")
    public List<SensorReadingDTO> retrieveSensorReadings() {

        return sensorService.retrieveSensorReadings();
    }

    @ApiOperation(value = "Retrieves Latest SensorReading", response = SensorReadingDTO.class, httpMethod = "GET")
    @ResponseBody
    @GetMapping("/latest")
    public ResponseEntity<SensorReadingDTO> retrieveLatestSensorReading() {
        return sensorService.getLatestSensorReading()
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.noContent().build());
    }

}