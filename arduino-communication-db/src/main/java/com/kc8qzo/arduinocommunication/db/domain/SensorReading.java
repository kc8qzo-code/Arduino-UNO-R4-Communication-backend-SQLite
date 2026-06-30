package com.kc8qzo.arduinocommunication.db.domain;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

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
}