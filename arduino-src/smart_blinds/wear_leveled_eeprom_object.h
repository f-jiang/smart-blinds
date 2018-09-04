#ifndef WEAR_LEVELED_EEPROM_OBJECT
#define WEAR_LEVELED_EEPROM_OBJECT

#include <stddef.h>
#include <stdint.h>

#include <EEPROM.h>

#define EEPROM_SIZE_BYTES 1024  // TODO determine size based on board type

template<class T>
class WearLeveledEepromObject {
public:

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
                break;
            }

            a = b;
        }
    }

    T& get(T& object) const
    {
        CircularQueueItem retrievedItem;
        return EEPROM.get(m_curAddr, retrievedItem).value;
    }

    T& put(T& object)
    {
        uint16_t nextPos = m_curItem.position + 1;
        if (nextPos == UINT16_MAX) {
            nextPos = 0;
        }

        uint16_t nextAddr = m_curAddr + m_circularQueueItemSize;
        if (nextAddr == m_endAddr) {
            nextAddr = m_startAddr;
        }

        CircularQueueItem newItem;
        newItem.position = nextPos;
        newItem.value = object;

        return EEPROM.put(nextAddr, newItem).value;
    }

private:

    struct CircularQueueItem {
        uint16_t position;
        T value;

        static bool sequential(const CircularQueueItem& a, const CircularQueueItem& b)
        {
            return ((b.position - a.position) == 1) ||
                   ((a.position == (UINT16_MAX - 1)) && (a.position == 0));
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

