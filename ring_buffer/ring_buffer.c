/*
 * ring_buffer.c
 *
 *  Created on: Apr 5, 2024
 *      Author: Am0k
 */

#include "ring_buffer.h"

void RING_BUF_Init(RING_buffer_t* Handle, uint8_t size)
{
    Handle->_m_size = size;
    //Handle->_ADC_RW_array = new uint16_t[size];
    Handle->_ADC_RW_array = malloc(size*sizeof(uint16_t));
    memset(Handle->_ADC_RW_array, 0, size * sizeof(uint16_t));
    Handle->_m_index = 0;
    Handle->_m_full = false;
}

void RING_BUF_Push(RING_buffer_t* Handle, uint16_t value)
{
    Handle->_ADC_RW_array[Handle->_m_index] = value;
    if (Handle->_m_index < Handle->_m_size - 1)
    {
        Handle->_m_index++;
    }
    else
    {
        Handle->_m_index = 0;
        Handle->_m_full = true;
    }
}

uint16_t RING_BUF_getFiltered (RING_buffer_t* Handle, uint8_t maxDiffPercent)
{
    float average = 0;
    for (uint8_t currentIndex = 0; currentIndex < Handle->_m_size; currentIndex++)
    {
        average += Handle->_ADC_RW_array[currentIndex];
    }
    average = average / Handle->_m_size;

    float summ = 0;
    uint8_t members = 0;

    for (uint8_t currentIndex = 0; currentIndex < Handle->_m_size; currentIndex++)
    {
        float diff = abs (Handle->_ADC_RW_array[currentIndex] - average);
        if (diff < average * maxDiffPercent / 100)
        {
            summ += Handle->_ADC_RW_array[currentIndex];
            members++;
        }
    }
    if (members != 0)
    {
        return round(summ / members);
    }
    else
    {
        return round(average);
    }
}
