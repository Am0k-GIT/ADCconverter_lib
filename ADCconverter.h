#pragma once

#include <data_statistic.h>                                                     // библиотека для использования кольцевого буффера

typedef uint16_t raw_value_t;                                                   // тип сырых показаний АЦП
typedef int16_t proc_value_t;                                                   // тип конвертированных в градусы Цельсия показаний АЦП

typedef struct { raw_value_t raw_value; proc_value_t proc_value; } calibration_entry_t;
                                                                                // структура соответствующих пар сырых показаний / реальных значений

#include "tables.h"

class ADCconverter
{
private:

    RingBuffer<raw_value_t, uint8_t>* buffer;                                   // кольцевой буфер для хранения сырых показаний АЦП
    int16_t _pin;                                                               // аналоговый вход МК

public:
    ADCconverter(uint8_t buffer_size)                                           // конструктор
    {
        buffer = new RingBuffer<raw_value_t, uint8_t>(buffer_size);

    }

    ~ADCconverter()                                                             // деструктор
    {
        delete buffer;
    }

    void begin(const uint8_t pin)                                               // инициализация
    {
        _pin = pin;
        analogReadResolution(12);
        pinMode(_pin, INPUT_ANALOG);
    }

    raw_value_t read()                                                          // чтение показаний АЦП в кольцевой буфер
    {
        raw_value_t value = analogRead(_pin);
        buffer->push(value);
        return value;
    }

    raw_value_t getFiltered()                                                   // получение отфильтрованного сырого значения
    {
        return buffer->getFiltered(medianValue, 10);
    }

    proc_value_t getValue()                                                     // получение конвертированного значения
    {
        uint8_t left_index = 0;                                                 // самый левый индекс калибровочной таблицы
        uint8_t right_index = sizeof(CALIBRATION_TABLE)/sizeof((CALIBRATION_TABLE)[0]) - 1;
        bool ascending_sort = false;
        if (pgm_read_word(&CALIBRATION_TABLE[left_index].raw_value) < pgm_read_word(&CALIBRATION_TABLE[right_index].proc_value))
            ascending_sort = true;
        raw_value_t current_raw_value = ADCconverter::getFiltered();            // получаем сырые отфильтрованные показания АЦП
        if (ascending_sort)
        {
            if (current_raw_value <= pgm_read_word(&CALIBRATION_TABLE[left_index].raw_value))
                                                                                // проверяем левый предел
                return pgm_read_word(&CALIBRATION_TABLE[left_index].proc_value);            
                                                                                // подгоняем значение под крайнее левое
            if (current_raw_value >= pgm_read_word(&CALIBRATION_TABLE[right_index].raw_value))       
                                                                                // проверяем правый предел
                return pgm_read_word(&CALIBRATION_TABLE[right_index].proc_value);           
                                                                                // подгоняем значение под крайнее правое
        }
        else
        {
            if (current_raw_value >= pgm_read_word(&CALIBRATION_TABLE[left_index].raw_value))       
                                                                                // проверяем левый предел
                return pgm_read_word(&CALIBRATION_TABLE[left_index].proc_value);            
                                                                                // подгоняем значение под крайнее левое
            if (current_raw_value <= pgm_read_word(&CALIBRATION_TABLE[right_index].raw_value))       
                                                                                // проверяем правый предел
                return pgm_read_word(&CALIBRATION_TABLE[right_index].proc_value);
                                                                                // подгоняем значение под крайнее правое
        }
        while ((right_index - left_index) > 1)                                  // пока не получим минимальный интервал в таблице
        {
            uint8_t mid_index = (left_index + right_index) >> 1;                // находим середину
            raw_value_t mid = pgm_read_word(&CALIBRATION_TABLE[mid_index].raw_value);
                                                                                // читаем среднее сырое значение из таблицы

            if (((current_raw_value > mid) && (!ascending_sort)) || ((current_raw_value < mid) && (ascending_sort)))                                                 // если показания АЦП больше среднего значения из таблицы
            {
                right_index = mid_index;                                        // смещаем правый индекс
            } 
            else                                                                // иначе
            {
                left_index = mid_index;                                         // смещаем левый индекс
            }
        }                                                                       // теперь в минимальное возможном интервале
        raw_value_t vl = pgm_read_word(&CALIBRATION_TABLE[left_index].raw_value);    
                                                                                // читаем из таблицы левое сырое значение
        proc_value_t vl_proc = pgm_read_word(&CALIBRATION_TABLE[left_index].proc_value);
                                                                                // и соответствующее конвертированное значение
        if (((current_raw_value >= vl) && (!ascending_sort)) || ((current_raw_value <= vl) && (ascending_sort)))                                                       // если показания АЦП больше или равны левому сырому значению
        {
            return vl_proc;                                                     // возвращаем соответствующее конвертированное значение
        }
        raw_value_t vr = pgm_read_word(&CALIBRATION_TABLE[right_index].raw_value);
                                                                                // читаем из таблицы правое сырое значение
        raw_value_t vd = abs(vl - vr);                                          // подсчитываем разность сырых значений
        proc_value_t res = pgm_read_word(&CALIBRATION_TABLE[right_index].proc_value);
                                                                                // в результат записываем правое конвертированное значение
        if (vd)                                                                 // если  разность сырых значений не нулевая
        {
            res -= round((res - vl_proc) * abs(current_raw_value - vr) / vd);   // проводим линейную интерполяцию для поиска смещения конвертированного значения
        }                                                                       // и вычитаем из результата
        return res;                                                             // возвращаем результат
    }
};