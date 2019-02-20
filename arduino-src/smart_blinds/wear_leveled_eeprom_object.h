#ifndef WEAR_LEVELED_EEPROM_OBJECT
#define WEAR_LEVELED_EEPROM_OBJECT

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <EEPROM.h>

#define EEPROM_SIZE_BYTES  1024  // TODO determine size based on board type
#define POSITION_UNDEFINED UINT16_MAX

template<class T>
class WearLeveledEepromObject {
public:

    static size_t circularQueueItemSize() {
        return sizeof(CircularQueueItem);
    }

    WearLeveledEepromObject(uint16_t startAddr, size_t circularQueueItemCount) :
        m_startAddr(startAddr),
        m_circularQueueItemCount(circularQueueItemCount),
        m_circularQueueItemSize(sizeof(CircularQueueItem))
    {
        if (indexToAddress(m_circularQueueItemCount) > EEPROM_SIZE_BYTES) {
            m_circularQueueItemCount = (EEPROM_SIZE_BYTES - startAddr) / m_circularQueueItemSize;
        }

        m_endAddr = indexToAddress(m_circularQueueItemCount);

        CircularQueueItem a, b;
        EEPROM.get(m_startAddr, a);
        for (size_t i = 0; i < (m_circularQueueItemCount - 1); i++) {
            EEPROM.get(indexToAddress(i + 1), b);

            if (!CircularQueueItem::sequential(a, b)) {
                m_curAddr = indexToAddress(i);
                m_curItem = a;

                if (m_curItem.position == POSITION_UNDEFINED) {
                    memset(&m_curItem.value, 0xFF, sizeof(T));
                    m_curItem.position = 0;
                    EEPROM.put(m_curAddr, m_curItem);
                    Serial.println("initializing first circular queue item");
                }

                break;
            }

            a = b;
        }
    }

    WearLeveledEepromObject() = delete;

    T& get(T& object) const
    {
        CircularQueueItem retrievedItem;
        EEPROM.get(m_curAddr, retrievedItem);
        object = retrievedItem.value;
        return object;
    }

    T& put(T& object)
    {
        uint16_t nextPos = m_curItem.position + 1;
        if (nextPos == POSITION_UNDEFINED) {
            nextPos = 0;
        }

        uint16_t nextAddr = m_curAddr + m_circularQueueItemSize;
        if (nextAddr == m_endAddr) {
            nextAddr = m_startAddr;
        }

        CircularQueueItem newItem;
        newItem.position = nextPos;
        newItem.value = object;

        EEPROM.put(nextAddr, newItem);
        m_curAddr = nextAddr;
        m_curItem = newItem;

        return object;
    }

private:

    struct CircularQueueItem {
        uint16_t position;
        T value;

        static bool sequential(const CircularQueueItem& a, const CircularQueueItem& b)
        {
            return ((b.position - a.position) == 1) ||
                   ((a.position == (POSITION_UNDEFINED - 1)) && (a.position == 0));
        }
    };

    uint16_t m_startAddr;
    uint16_t m_endAddr; // address that comes after last item in queue
    uint16_t m_curAddr;
    CircularQueueItem m_curItem;
    size_t m_circularQueueItemCount;
    size_t m_circularQueueItemSize;

    uint16_t indexToAddress(size_t index) const
    {
        return m_startAddr + (index * m_circularQueueItemSize);
    }

};

#endif  // WEAR_LEVELED_EEPROM_OBJECT

