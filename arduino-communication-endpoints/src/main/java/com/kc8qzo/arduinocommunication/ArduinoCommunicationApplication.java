package com.kc8qzo.arduinocommunication;

import org.modelmapper.ModelMapper;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;

@SpringBootApplication
public class ArduinoCommunicationApplication {
    public static void main(String[] args) {
        SpringApplication.run(ArduinoCommunicationApplication.class, args);
    }

    @Bean
    ModelMapper modelMapper() {
        return new ModelMapper();
    }
}
