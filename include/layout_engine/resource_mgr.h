#include <stdint.h>
#include <windows.h>

typedef struct {
    uint16_t thread;
    uint16_t core;
} PhysicalDeviceThreadInformation;
typedef struct {
    uint16_t logical_core_count;
    uint16_t physical_core_count;
    PhysicalDeviceThreadInformation physical_device_core_n_thread_relationship[512];
} WorkersProperties;

void WorkersGetProperties(WorkersProperties *wp)
{
    uint16_t max;
    ARR_SIZE(&max, UINT16_MAX, wp->physical_device_core_n_thread_relationship);

    DWORD len;
    GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info = malloc(len);
    if (!info) return;

    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &len))
    {
        free(info);
        return;
    }
    uint16_t count = 0;
    uint16_t physical_core = 0;

    BYTE *p = (BYTE *)info;
    BYTE *end = p + len;

    while (p < end)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX e = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)p;

        if (e->Relationship == RelationProcessorCore)
        {
            KAFFINITY mask = e->Processor.GroupMask[0].Mask;

            for (uint16_t bit = 0; bit < sizeof(KAFFINITY) * 8; bit++)
            {
                if (mask & ((KAFFINITY)1 << bit))
                {
                    if (count < max)
                    {
                        wp->physical_device_core_n_thread_relationship[count].thread = bit;
                        wp->physical_device_core_n_thread_relationship[count].core = physical_core;
                    }
                    count++;
                }
            }
            physical_core++;
        }
        p += e->Size;
    }
    free(info);
    wp->logical_core_count = count;
    wp->physical_core_count = physical_core;

}