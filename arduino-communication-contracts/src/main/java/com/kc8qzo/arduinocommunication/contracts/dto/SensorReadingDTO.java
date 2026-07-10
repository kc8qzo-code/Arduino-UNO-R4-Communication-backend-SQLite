package com.kc8qzo.arduinocommunication.contracts.dto;

import io.swagger.annotations.ApiModelProperty;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

import java.time.Instant;

@Getter
@Setter
@EqualsAndHashCode
@ToString
public class SensorReadingDTO {
    @ApiModelProperty(value = "temperature", example = "5.55")
    private double temperature;

    @ApiModelProperty(value = "humidity", example = "5.52")
    private double humidity;

    @ApiModelProperty(value = "light", example = "5")
    private Integer light;

    @ApiModelProperty(value = "passValue", example = "123456")
    private long passValue;

    private Instant postedAt;
}
