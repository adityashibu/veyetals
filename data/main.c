#ifdef _WIN32
#include <windows.h>
#include <Psapi.h>
#elif __linux__
#include <unistd.h>
#endif

#include <stdio.h>
#include <nvml.h>

void get_memory_usage()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    printf("Memory Usage: %ld%%\n", memInfo.dwMemoryLoad);
    printf("Total Physical Memory: %lld MB\n", memInfo.ullTotalPhys / (1024 * 1024));
    printf("Available Physical Memory: %lld MB\n", memInfo.ullAvailPhys / (1024 * 1024));
#elif __linux__
    // Add linux code here
#endif
}

void get_cpu_usage()
{
    FILETIME idleTime, kernelTime, userTime;
    static FILETIME prev_idleTime = {0}, prev_kernelTime = {0}, prev_userTime = {0};

    if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        ULARGE_INTEGER idle, kernel, user;
        ULARGE_INTEGER prev_idle, prev_kernel, prev_user;
        ULARGE_INTEGER total_time, prev_total_time;
        double cpu_usage;

        idle.QuadPart = ((ULARGE_INTEGER *)&idleTime)->QuadPart;
        kernel.QuadPart = ((ULARGE_INTEGER *)&kernelTime)->QuadPart;
        user.QuadPart = ((ULARGE_INTEGER *)&userTime)->QuadPart;

        prev_idle.QuadPart = ((ULARGE_INTEGER *)&prev_idleTime)->QuadPart;
        prev_kernel.QuadPart = ((ULARGE_INTEGER *)&prev_kernelTime)->QuadPart;
        prev_user.QuadPart = ((ULARGE_INTEGER *)&prev_userTime)->QuadPart;

        ULONGLONG idle_diff = idle.QuadPart - prev_idle.QuadPart;
        ULONGLONG kernel_diff = kernel.QuadPart - prev_kernel.QuadPart;
        ULONGLONG user_diff = user.QuadPart - prev_user.QuadPart;

        ULONGLONG total_diff = kernel_diff + user_diff;
        ULONGLONG total_idle = idle_diff;

        if (total_diff == 0)
        {
            cpu_usage = 0;
        }
        else
        {
            cpu_usage = (double)(total_diff - total_idle) / total_diff * 1000.0;
        }

        printf("CPU Usage: %.1f%%\n", cpu_usage);

        prev_idleTime = idleTime;
        prev_kernelTime = kernelTime;
        prev_userTime = userTime;
    }
}

void get_gpu_usage()
{
    nvmlReturn_t result;
    nvmlDevice_t device;

    result = nvmlInit();
    if (result != NVML_SUCCESS)
    {
        printf("Failed to initialize NVML: %s\n", nvmlErrorString(result));
        return;
    }

    result = nvmlDeviceGetHandleByIndex_v2(0, &device);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get handle for device: %s\n", nvmlErrorString(result));
        nvmlShutdown();
        return;
    }

    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    result = nvmlDeviceGetName(device, name, sizeof(name));
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU Name: %s\n", nvmlErrorString(result));
        nvmlShutdown();
        return;
    }

    printf("GPU Name: %s\n", name);

    nvmlUtilization_t utilization;
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU utilization rates: %s\n", nvmlErrorString(result));
    }
    else
    {
        printf("GPU Usage: %u%%\n", utilization.gpu);
    }

    nvmlMemory_t memory;
    result = nvmlDeviceGetMemoryInfo(device, &memory);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU memory info: %s\n", nvmlErrorString(result));
    }
    else
    {
        printf("GPU Memory Usage: %llu MB / %llu MB\n", memory.used / (1024 * 1024), memory.total / (1024 * 1024));
    }

    nvmlShutdown();
}

int main()
{
    while (1)
    {
        get_memory_usage();
        get_cpu_usage();
        get_gpu_usage();
        Sleep(1000);
        printf("--------------------------------------------\n");
    }
    return 0;
}