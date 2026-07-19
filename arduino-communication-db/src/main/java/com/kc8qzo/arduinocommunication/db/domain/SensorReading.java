package com.kc8qzo.arduinocommunication.db.domain;

import jakarta.persistence.*;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

import java.time.Instant;

@Entity
@Getter
@Setter
@EqualsAndHashCode
@Table(name = "arduino_sensor")
public class SensorReading {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY) // ✓ works in SQLite
    private Long id;

    @Column
    private double temperature;

    @Column
    private double humidity;

    @Column
    private Integer light;

    public Integer getMyInteger() {
        return light;
    }

    public void setMyInteger(Integer light) {
        this.light = light;
    }

    @Column
    private long passValue;

    @Column
    private Instant sentAt;

    @Column(nullable = false)
    private Instant postedAt;
}