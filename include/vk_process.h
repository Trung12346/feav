#ifndef VK_PROCESS_H
#define VK_PROCESS_H



#include <vulkan/vulkan.h>
#include <stdint.h>
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <string.h>
#include "debugger.h"
#include "main.h"

#define VALIDATION_LAYER_COUNT 1
const char *validation_layers[VALIDATION_LAYER_COUNT] = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool validation_layers_enabled = false;
#else
const bool validation_layers_enabled = true;
#endif

typedef struct
{
    VkInstance *instance;
    VkDebugUtilsMessengerEXT debug_message;
    VkPhysicalDevice *physical_device;
    VkDevice *logical_device;
    VkQueue *graphics_queue;
    uint32_t graphics_queue_index;
    VkSurfaceKHR *surface;
    VkSwapchainKHR *swapchain;
    VkExtent2D *extent_2d;
    VkSurfaceFormatKHR *surface_format;
    uint32_t image_count;
    VkImage *images;
    uint32_t image_view_count;
    VkImageView *image_views;
    VkPipelineLayout *pipeline_layout;
    VkPipeline *graphics_pipeline;
    VkCommandPool *command_pool;
    uint32_t command_buffer_count;
    VkCommandBuffer *command_buffers;
    VkSemaphore *present_complete_semaphore;
    VkSemaphore *render_finished_semaphore;
    VkFence *fence;
    
} VkProcess;

VkProcess vk_process_no_args_construct(void)
{
    return (VkProcess)
    {
        .instance = NULL,
        .physical_device = NULL,
        .logical_device = NULL,
        .graphics_queue = NULL,
        .surface = NULL,
        .swapchain = NULL,
        .extent_2d = NULL,
        .surface_format = NULL,
        .images = NULL,
        .image_views = NULL,
        .pipeline_layout = NULL,
        .graphics_pipeline = NULL,
        .command_pool = NULL,
        .command_buffers = NULL,
        .present_complete_semaphore = NULL,
        .render_finished_semaphore = NULL,
        .fence = NULL
    };
}
static bool enumerate_extension_properties_check(void *properties, uint32_t extension_count, const char *requirement)
{
    VkExtensionProperties *extensions = properties;
    for (uint32_t x = 0U; x < extension_count; x++)
    {
        if (strcmp(requirement, extensions[x].extensionName) == 0)
        {
            return true;
        }
    }
    return false;
}
static bool enumerate_callback
(
    char **requirements,
    uint32_t requirement_count,
    void *properties,
    uint32_t prop_count,
    bool (*func)(void *props, uint32_t cnt, const char *requirement)
)
{
    for (uint32_t i = 0; i < requirement_count; i++)
    {
        if (!func(properties, prop_count, requirements[i])) return false;
    }
    return true;
}
void instance_create(VkProcess *process)
{   
    //VkInstance *vk_instance = &process->instance;
    

    const VkApplicationInfo app_info = (VkApplicationInfo)
    {
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = VK_API_VERSION_1_4,
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO
    };



    const char *required_layers[VALIDATION_LAYER_COUNT];
    if (validation_layers_enabled)
    {
        printf(INF VK_DBG_PREFIX" Validation layer enabled\n");
        memcpy(required_layers, validation_layers, sizeof(validation_layers));
    }
    else
    {
        printf(INF VK_DBG_PREFIX" Validation layer disabled\n");
    }
    uint32_t vk_layer_count;
    vkEnumerateInstanceLayerProperties(&vk_layer_count, NULL);
    printf(INF VK_DBG_PREFIX" found %u layer(s)\n", vk_layer_count);
    VkLayerProperties *vk_layers = malloc(sizeof(VkLayerProperties) * vk_layer_count);
    vkEnumerateInstanceLayerProperties(&vk_layer_count, vk_layers);
    for (uint32_t i = 0U; i < vk_layer_count; i++)
    {
        printf(INF VK_DBG_PREFIX" found %s ver_%u\n", vk_layers[i].layerName, vk_layers[i].specVersion);
    }

    for (uint32_t i = 0U; i < VALIDATION_LAYER_COUNT; i++)
    {
        bool match = false;
        for (uint32_t x = 0; x < vk_layer_count; x++)
        {
            if (strcmp(required_layers[i], vk_layers[x].layerName) == 0)
            {
                match = true;
                break;
            }
        }
        if (!match)
        {
            printf(ERR VK_DBG_PREFIX" Could not find required %s layer from Vulkan, unable to proceed\n", required_layers[i]);
            exit(1);
        }
    }


    uint32_t glfw_extension_count;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    const char *additional_extensions[] =
    {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    uint32_t additional_extension_count = sizeof(additional_extensions) / sizeof(additional_extensions[0]);

    uint32_t required_extension_count = glfw_extension_count + additional_extension_count;
    const char *required_extensions[required_extension_count];
    memcpy(required_extensions, glfw_extensions, sizeof(glfw_extensions[0]) * glfw_extension_count);
    memcpy(&required_extensions[glfw_extension_count], additional_extensions, sizeof(additional_extensions));

    printf(INF GLFW_DBG_PREFIX" requires: %u extension(s)\n", required_extension_count);
    for (uint32_t i = 0U; i < required_extension_count; i++)
    {
        printf(INF GLFW_DBG_PREFIX" requires %s\n", required_extensions[i]);
    }

    uint32_t vk_extension_count;
    vkEnumerateInstanceExtensionProperties(NULL, &vk_extension_count, NULL);
    printf(INF VK_DBG_PREFIX" found:  %u extension(s)\n", vk_extension_count);
    VkExtensionProperties *vk_extensions = malloc(sizeof(VkExtensionProperties) * vk_extension_count);
    vkEnumerateInstanceExtensionProperties(NULL, &vk_extension_count, vk_extensions);
    for (uint32_t i = 0U; i < vk_extension_count; i++)
    {
        printf(INF VK_DBG_PREFIX" found %s ver_%u\n", vk_extensions[i].extensionName, vk_extensions[i].specVersion);
    }

    for (uint32_t i = 0U; i < required_extension_count; i++)
    {
        
        if (!enumerate_extension_properties_check(vk_extensions, vk_extension_count, required_extensions[i]))
        {
            printf(ERR GLFW_DBG_PREFIX" Could not find required %s extension from Vulkan, unable to proceed\n", required_extensions[i]);
            exit(1);
        }
    }

    VkInstanceCreateInfo info_create = (VkInstanceCreateInfo)
    {
        .pApplicationInfo = &app_info,
        .enabledLayerCount = validation_layers_enabled ? VALIDATION_LAYER_COUNT : 0,
        .ppEnabledLayerNames = validation_layers_enabled ? required_layers : NULL,
        .enabledExtensionCount = required_extension_count,
        .ppEnabledExtensionNames = required_extensions,
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
    };

    process->instance = malloc(sizeof(VkInstance));
    if (vkCreateInstance(&info_create, NULL, process->instance) != VK_SUCCESS)
    {
        printf(ERR VK_DBG_PREFIX" Vulkan instance created unsuccessfully, unable to proceed\n");
        exit(1);
    }
    
    printf(INF VK_DBG_PREFIX" Vulkan instane created successfully\n");

    free(vk_extensions);
    free(vk_layers);
}
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *
)
{
	if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT || severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		printf(INF VK_DBG_PREFIX" Validation layer: type %u msg: %s\n", type, pCallbackData->pMessage);
	}

	return VK_FALSE;
}
void messenger_debug_setup(VkProcess *process)
{
    if (!validation_layers_enabled)
    {
        return;
    }

    VkDebugUtilsMessageSeverityFlagsEXT severity_flags =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    ;
    VkDebugUtilsMessageTypeFlagsEXT message_type_flags =
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    ;
    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info_ext = (VkDebugUtilsMessengerCreateInfoEXT)
    {
        .messageSeverity = severity_flags,
        .messageType = message_type_flags,
        .pfnUserCallback = &debugCallback,
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
    };

    PFN_vkCreateDebugUtilsMessengerEXT wrapperCreateDebugUtilsMessengerEXT =
    (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*process->instance, "vkCreateDebugUtilsMessengerEXT");

    if (!wrapperCreateDebugUtilsMessengerEXT)
    {
        printf(WARN VK_DBG_PREFIX" Failed loading wrapper for instance\n");
    }
    else
    {
        wrapperCreateDebugUtilsMessengerEXT
        (
            *process->instance,
            &debug_utils_messenger_create_info_ext,
            NULL,
            &process->debug_message
        );
    }
    
}
static char *vk_physical_device_type_resolver(VkPhysicalDeviceType bit_flag)
{
    switch (bit_flag)
    {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "unidentified";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "integrated gpu";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "discrete gpu";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "virtual gpu";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "cpu";
    }
}
static char *vk_bool_resolver(VkBool32 vk_bool)
{
    return vk_bool ? "true" : "false";
}
void physical_device_pick(VkProcess *process, int device_select)
{
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(*process->instance, &physical_device_count, NULL);

    if (physical_device_count == 0U)
    {
        printf(ERR VK_DBG_PREFIX" Device does not support Vulkan, unable to proceed\n");
        exit(1);
    }
    printf(INF VK_DBG_PREFIX" Found %u physical device(s)\n", physical_device_count);

    VkPhysicalDevice *vk_physical_devices = malloc(sizeof(VkPhysicalDevice) * physical_device_count);
    vkEnumeratePhysicalDevices(*process->instance, &physical_device_count, vk_physical_devices);

    if (device_select <= -1)
    {
        VkPhysicalDeviceProperties vk_physical_device_properties;
        VkPhysicalDeviceFeatures vk_physical_device_features;
        char *required_extensions[] = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        uint32_t requirement_count = sizeof(required_extensions) / sizeof(required_extensions[0]);

        uint32_t (*candidates)[2] = calloc(physical_device_count, sizeof(*candidates));
        uint32_t candidate_index = 0;

        for (uint32_t i = 0U; i < physical_device_count; i++)
        {
            uint32_t device_extension_count;
            vkGetPhysicalDeviceProperties(vk_physical_devices[i], &vk_physical_device_properties);
            vkGetPhysicalDeviceFeatures(vk_physical_devices[i], &vk_physical_device_features);
            vkEnumerateDeviceExtensionProperties(vk_physical_devices[i], NULL, &device_extension_count, NULL);
            VkExtensionProperties device_extensions[device_extension_count];
            vkEnumerateDeviceExtensionProperties(vk_physical_devices[i], NULL, &device_extension_count, device_extensions);

            uint32_t score = 0U;

            if (vk_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 1000U;
            }
            score += vk_physical_device_properties.limits.maxImageDimension2D;
            if
            (
                vk_physical_device_features.geometryShader &&
                enumerate_callback(required_extensions, requirement_count, device_extensions, device_extension_count, &enumerate_extension_properties_check)
            )
            {
                candidates[candidate_index][0] = score;
                candidates[candidate_index][1] = i;
                candidate_index++;
            }

            printf(INF VK_DBG_PREFIX" Found device #%u:\n", i);
            printf(TAB"api version: %u\n", vk_physical_device_properties.apiVersion);
            printf(TAB"driver version: %u\n", vk_physical_device_properties.driverVersion);
            printf(TAB"vendor id: %u\n", vk_physical_device_properties.vendorID);
            printf(TAB"device id: %u\n", vk_physical_device_properties.deviceID);
            printf(TAB"device type: %s\n", vk_physical_device_type_resolver(vk_physical_device_properties.deviceType));
            printf(TAB"device name: %s\n", vk_physical_device_properties.deviceName);
            printf(TAB"pipeline cache uuid: %u\n", vk_physical_device_properties.pipelineCacheUUID);

            VkPhysicalDeviceLimits limits = vk_physical_device_properties.limits;
            printf(TAB"limits:\n");
            printf(TAB TAB"max 1d image: %u\n", limits.maxImageDimension1D);
            printf(TAB TAB"max 2d image: %u\n", limits.maxImageDimension2D);
            printf(TAB TAB"max 3d image: %u\n", limits.maxImageDimension3D);
            printf(TAB TAB"max cube image: %u\n", limits.maxImageDimensionCube);
            printf(TAB TAB"max image array layer: %u\n", limits.maxImageArrayLayers);
            printf(TAB TAB"max texel buffer elements: %u\n", limits.maxTexelBufferElements);
            printf(TAB TAB"max uniform buffer range: %u\n", limits.maxUniformBufferRange);
            printf(TAB TAB"max storage buffer range: %u\n", limits.maxStorageBufferRange);
            printf(TAB TAB"max push constants size: %u\n", limits.maxPushConstantsSize);
            printf(TAB TAB"max memory: %u\n", limits.maxMemoryAllocationCount);
            printf(TAB TAB"max sample allocation count: %u\n", limits.maxSamplerAllocationCount);
            printf(TAB TAB"image buffer granularity: %u\n", limits.bufferImageGranularity);
            printf(TAB TAB"sparse address space size: %u\n", limits.sparseAddressSpaceSize);
            printf(TAB TAB"max bound descriptor set: %u\n", limits.maxBoundDescriptorSets);
            printf(TAB TAB"max psd sampler: %u\n", limits.maxPerStageDescriptorSamplers);
            printf(TAB TAB"max psd uniform buffer: %u\n", limits.maxPerStageDescriptorUniformBuffers);
            printf(TAB TAB"max psd storage buffer: %u\n", limits.maxPerStageDescriptorStorageBuffers);
            printf(TAB TAB"max psd sampled image: %u\n", limits.maxPerStageDescriptorSampledImages);
            printf(TAB TAB"max psd storage image: %u\n", limits.maxPerStageDescriptorStorageImages);
            printf(TAB TAB"max psd input attachment: %u\n", limits.maxPerStageDescriptorInputAttachments);
            printf(TAB TAB"max ps resource: %u\n", limits.maxPerStageResources);
            printf(TAB TAB"max descriptor set sampler: %u\n", limits.maxDescriptorSetSamplers);
            printf(TAB TAB"max descriptor set uniform buffer: %u\n", limits.maxDescriptorSetUniformBuffers);
            printf(TAB TAB"max descriptor uniform buffer dynamic: %u\n", limits.maxDescriptorSetUniformBuffersDynamic);
            printf(TAB TAB"max descriptor set storage buffer: %u\n", limits.maxDescriptorSetStorageBuffers);
            printf(TAB TAB"max descriptor set storage bubffer dynamic: %u\n", limits.maxDescriptorSetStorageBuffersDynamic);
            printf(TAB TAB"max descriptor set sampled image: %u\n", limits.maxDescriptorSetSampledImages);
            printf(TAB TAB"max descriptor set storage image: %u\n", limits.maxDescriptorSetStorageImages);
            printf(TAB TAB"max descriptor set input attachment: %u\n", limits.maxDescriptorSetInputAttachments);
            printf(TAB TAB"max vertex input attribute: %u\n", limits.maxVertexInputAttributes);
            printf(TAB TAB"max vertex input binding: %u\n", limits.maxVertexInputBindings);
            printf(TAB TAB"max vertex input attribute offset: %u\n", limits.maxVertexInputAttributeOffset);
            printf(TAB TAB"max vertex input binding stride: %u\n", limits.maxVertexInputBindingStride);
            printf(TAB TAB"max vertex output component: %u\n", limits.maxVertexOutputComponents);
            printf(TAB TAB"max tessellation generation level: %u\n", limits.maxTessellationGenerationLevel);
            printf(TAB TAB"max tessellation patch size: %u\n", limits.maxTessellationPatchSize);
            printf(TAB TAB"max TC per vertex input component: %u\n", limits.maxTessellationControlPerVertexInputComponents);
            printf(TAB TAB"max TC per vextex output component: %u\n", limits.maxTessellationControlPerVertexOutputComponents);
            printf(TAB TAB"max TC per patch output component: %u\n", limits.maxTessellationControlPerPatchOutputComponents);
            printf(TAB TAB"max TC total output component: %u\n", limits.maxTessellationControlTotalOutputComponents);
            printf(TAB TAB"max tessellation evaluation input component: %u\n", limits.maxTessellationEvaluationInputComponents);
            printf(TAB TAB"max tessellation evaluation output component: %u\n", limits.maxTessellationEvaluationOutputComponents);
            printf(TAB TAB"max geometry shader invocation: %u\n", limits.maxGeometryShaderInvocations);
            printf(TAB TAB"max geometry input component: %u\n", limits.maxGeometryInputComponents);
            printf(TAB TAB"max geometry output component: %u\n", limits.maxGeometryOutputComponents);
            printf(TAB TAB"max geometry output vertex: %u\n", limits.maxGeometryOutputVertices);
            printf(TAB TAB"max geometry total output component: %u\n", limits.maxGeometryTotalOutputComponents);
            printf(TAB TAB"max fragment input component: %u\n", limits.maxFragmentInputComponents);
            printf(TAB TAB"max fragment output attachment: %u\n", limits.maxFragmentOutputAttachments);
            printf(TAB TAB"max fragment dual src attachment: %u\n", limits.maxFragmentDualSrcAttachments);
            printf(TAB TAB"max fragment combined output resource: %u\n", limits.maxFragmentCombinedOutputResources);
            printf(TAB TAB"max compute shared memory size: %u\n", limits.maxComputeSharedMemorySize);
            printf(TAB TAB"max compute work group #0 count: %u\n", limits.maxComputeWorkGroupCount[0]);
            printf(TAB TAB"max compute work group #1 count: %u\n", limits.maxComputeWorkGroupCount[1]);
            printf(TAB TAB"max compute work group #2 count: %u\n", limits.maxComputeWorkGroupCount[2]);
            printf(TAB TAB"max compute work group invocation: %u\n", limits.maxComputeWorkGroupInvocations);
            printf(TAB TAB"max compute work group #0 size: %u\n", limits.maxComputeWorkGroupSize[0]);
            printf(TAB TAB"max compute work group #1 size: %u\n", limits.maxComputeWorkGroupSize[1]);
            printf(TAB TAB"max compute work group #2 size: %u\n", limits.maxComputeWorkGroupSize[2]);
            printf(TAB TAB"subpixel precision bit: %u\n", limits.subPixelPrecisionBits);
            printf(TAB TAB"subtexel precision bit: %u\n", limits.subTexelPrecisionBits);
            printf(TAB TAB"mipmap precision bit: %u\n", limits.mipmapPrecisionBits);
            printf(TAB TAB"max draw indexed index value: %u\n", limits.maxDrawIndexedIndexValue);
            printf(TAB TAB"max draw indirect count: %u\n", limits.maxDrawIndirectCount);
            printf(TAB TAB"max sampler lod bias: %f\n", limits.maxSamplerLodBias);
            printf(TAB TAB"max sampler anisotropy: %f\n", limits.maxSamplerAnisotropy);
            printf(TAB TAB"max viewport: %u\n", limits.maxViewports);
            printf(TAB TAB"max viewport width: %u\n", limits.maxViewportDimensions[0]);
            printf(TAB TAB"max viewport height: %u\n", limits.maxViewportDimensions[1]);
            printf(TAB TAB"max viewport bound range #0: %f\n", limits.viewportBoundsRange[0]);
            printf(TAB TAB"max viewport bound range #1: %f\n", limits.viewportBoundsRange[1]);
            printf(TAB TAB"viewport subpixel bit: %u\n", limits.viewportSubPixelBits);
            printf(TAB TAB"min memory map alignment: %u\n", limits.minMemoryMapAlignment);
            printf(TAB TAB"min texel buffer offset alignment: %u\n", limits.minTexelBufferOffsetAlignment);
            printf(TAB TAB"min uniform buffer offset alignment: %u\n", limits.minUniformBufferOffsetAlignment);
            printf(TAB TAB"min storage buffer offset alignment: %u\n", limits.minStorageBufferOffsetAlignment);
            printf(TAB TAB"min texel offset: %u\n", limits.minTexelOffset);
            printf(TAB TAB"max texel offset: %u\n", limits.maxTexelOffset);
            printf(TAB TAB"min texel gather offset: %u\n", limits.minTexelGatherOffset);
            printf(TAB TAB"max texel gather offset: %u\n", limits.maxTexelGatherOffset);
            printf(TAB TAB"min interpolation offset: %f\n", limits.minInterpolationOffset);
            printf(TAB TAB"max interpolation offset: %f\n", limits.maxInterpolationOffset);
            printf(TAB TAB"subpixel interpolation offset bit: %u\n", limits.subPixelInterpolationOffsetBits);
            printf(TAB TAB"max frame buffer width: %u\n", limits.maxFramebufferWidth);
            printf(TAB TAB"max frame buffer height: %u\n", limits.maxFramebufferHeight);
            printf(TAB TAB"max frame buffer layer: %u\n", limits.maxFramebufferLayers);
            printf(TAB TAB"frame buffer color sample count: %u\n", limits.framebufferColorSampleCounts);
            printf(TAB TAB"frame buffer depth sample count: %u\n", limits.framebufferDepthSampleCounts);
            printf(TAB TAB"frame buffer stencil sample count: %u\n", limits.framebufferStencilSampleCounts);
            printf(TAB TAB"frame buffer no attachment sample count: %u\n", limits.framebufferNoAttachmentsSampleCounts);
            printf(TAB TAB"max color attachment: %u\n", limits.maxColorAttachments);
            printf(TAB TAB"sampled image color sample count: %u\n", limits.sampledImageColorSampleCounts);
            printf(TAB TAB"sampled image integer sample count: %u\n", limits.sampledImageIntegerSampleCounts);
            printf(TAB TAB"sampled image depth sample count: %u\n", limits.sampledImageDepthSampleCounts);
            printf(TAB TAB"sampled image stencil sample count: %u\n", limits.sampledImageStencilSampleCounts);
            printf(TAB TAB"storage image sample count: %u\n", limits.storageImageSampleCounts);
            printf(TAB TAB"max sample mask word: %u\n", limits.maxSampleMaskWords);
            printf(TAB TAB"timestamp compute and graphics: %u\n", limits.timestampComputeAndGraphics);
            printf(TAB TAB"timestamp period: %f\n", limits.timestampPeriod);
            printf(TAB TAB"max clip distance: %u\n", limits.maxClipDistances);
            printf(TAB TAB"max cull distance: %u\n", limits.maxCullDistances);
            printf(TAB TAB"max combined clip and cull distance: %u\n", limits.maxCombinedClipAndCullDistances);
            printf(TAB TAB"discrete queue priority: %u\n", limits.discreteQueuePriorities);
            printf(TAB TAB"point size range #0: %f\n", limits.pointSizeRange[0]);
            printf(TAB TAB"point size range #1: %f\n", limits.pointSizeRange[1]);
            printf(TAB TAB"line width range #0: %f\n", limits.lineWidthRange[0]);
            printf(TAB TAB"line width range #1: %f\n", limits.lineWidthRange[1]);
            printf(TAB TAB"point size granularity: %f\n", limits.pointSizeGranularity);
            printf(TAB TAB"line width granularity: %f\n", limits.lineWidthGranularity);
            printf(TAB TAB"strict line: %s\n", vk_bool_resolver(limits.strictLines));
            printf(TAB TAB"standard sample location: %s\n", vk_bool_resolver(limits.standardSampleLocations));
            printf(TAB TAB"optimal buffer copy offset alignment: %u\n", limits.optimalBufferCopyOffsetAlignment);
            printf(TAB TAB"optimal buffer copy row pitch alignment: %u\n", limits.optimalBufferCopyRowPitchAlignment);
            printf(TAB TAB"non-coherent atom size: %u\n", limits.nonCoherentAtomSize);

            //printf(TAB"sparse properties: %\n", vk_physical_device_properties.);
        }

        if (candidate_index == 0U)
        {
            printf(ERR VK_DBG_PREFIX" Found no vulkan-compatible physical device, unable to proceed\n");
            exit(1);
        }
        uint32_t max_index = 0U;
        for (uint32_t i = 0U; i < candidate_index; i++)
        {
            if (candidates[i][0] > candidates[max_index][0])
            {
                max_index = i;
            }
        }
        
        device_select = max_index;
        free(candidates);
    }

    process->physical_device = malloc(sizeof(VkPhysicalDevice));
    *process->physical_device = vk_physical_devices[device_select];
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(*process->physical_device, &properties);
    printf(INF VK_DBG_PREFIX" Selected device: %s\n", properties.deviceName);
    printf(TAB"api version: %u\n", properties.apiVersion);
    printf(TAB"driver version: %u\n", properties.driverVersion);
    printf(TAB"vendor id: %u\n", properties.vendorID);
    printf(TAB"device id: %u\n", properties.deviceID);
    printf(TAB"device type: %s\n", vk_physical_device_type_resolver(properties.deviceType));
    printf(TAB"pipeline cache uuid: %u\n", properties.pipelineCacheUUID);
    
    free(vk_physical_devices);
}

void logical_device_create(VkProcess *process)
{
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(*process->physical_device, &queue_family_count, NULL);
    VkQueueFamilyProperties *vk_queue_families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(*process->physical_device, &queue_family_count, vk_queue_families);

    
    process->graphics_queue_index = queue_family_count;
    for (uint32_t i = 0U; i < queue_family_count; i++)
    {
        VkBool32 is_supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(*process->physical_device, i, *process->surface, &is_supported);
        if
        (
            (vk_queue_families[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)) &&
            is_supported
        )
        {
            process->graphics_queue_index = i;
            break;
        }
    }
    if (process->graphics_queue_index == queue_family_count)
    {
        printf(ERR VK_DBG_PREFIX" Physical device not compatible with requested resources, unable to proceed\n");
        exit(1);
    }

    float queue_prioriy = 1.0f;
    VkDeviceQueueCreateInfo device_queue_create_info = (VkDeviceQueueCreateInfo)
    {
        .queueFamilyIndex = process->graphics_queue_index,
        .queueCount = 1U,
        .pQueuePriorities = &queue_prioriy,
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT vk_physical_device_ext_feature = (VkPhysicalDeviceExtendedDynamicStateFeaturesEXT)
    {
        .extendedDynamicState = VK_TRUE,
        .pNext = NULL,
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT
    };
    VkPhysicalDeviceVulkan13Features vk_physical_device_vk13_feature = (VkPhysicalDeviceVulkan13Features)
    {
        .dynamicRendering = VK_TRUE,
        .synchronization2 = VK_TRUE,
        .pNext = &vk_physical_device_ext_feature,
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
    };
    VkPhysicalDeviceVulkan11Features vk_physical_device_vk11_feature = (VkPhysicalDeviceVulkan11Features)
    {
        .shaderDrawParameters = VK_TRUE,
        .pNext = &vk_physical_device_vk13_feature,
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
    };
    VkPhysicalDeviceFeatures2 vk_physical_device_feature2 = (VkPhysicalDeviceFeatures2)
    {
        .pNext = &vk_physical_device_vk11_feature,
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
    };

    const char *required_device_extensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    uint32_t required_device_extension_count = sizeof(required_device_extensions) / sizeof(required_device_extensions[0]);

    VkDeviceCreateInfo device_create_info = (VkDeviceCreateInfo)
    {
        .pNext = &vk_physical_device_feature2,
        .queueCreateInfoCount = 1U,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = required_device_extension_count,
        .ppEnabledExtensionNames = required_device_extensions,
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
    };

    process->logical_device = malloc(sizeof(VkDevice));
    vkCreateDevice(*process->physical_device, &device_create_info, NULL, process->logical_device);
    process->graphics_queue = malloc(sizeof(VkQueue));
    vkGetDeviceQueue(*process->logical_device, process->graphics_queue_index, 0U, process->graphics_queue);

    printf(INF VK_DBG_PREFIX" Device created successfully\n");

    free(vk_queue_families);
}

static VkExtent2D swap_extent_choose(VkSurfaceCapabilitiesKHR *surface_cap, GLFWwindow *window)
{
    if (surface_cap->currentExtent.width != UINT32_MAX) return surface_cap->currentExtent;

    uint32_t width, height = 0U;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D max_extent = surface_cap->maxImageExtent;
    VkExtent2D min_extent = surface_cap->minImageExtent;
    if (width > max_extent.width) width = max_extent.width;
    else if (width < min_extent.width) width = min_extent.width;
    if (height > max_extent.height) height = max_extent.height;
    else if (height < min_extent.height) height = min_extent.height;

    return (VkExtent2D) {width, height};
}
static uint32_t swap_min_image_count(VkSurfaceCapabilitiesKHR *surface_cap)
{
    uint32_t min_image_count = surface_cap->minImageCount > 3U ? surface_cap->minImageCount : 3U;

    if
    (
        (surface_cap->maxImageCount > 0U) &&
        (surface_cap->maxImageCount < min_image_count)
    ) return surface_cap->maxImageCount;
    return min_image_count;
}
static VkSurfaceFormatKHR swap_surface_format_choose(VkSurfaceFormatKHR *formats, uint32_t format_count)
{
    if (format_count < 1)
    {
        printf(ERR VK_DBG_PREFIX" No available surface format for this physical device, unable to proceed\n");
        exit(1);
    }
    for (uint32_t i = 0; i < format_count; i++)
    {
        if
        (
            formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        )
        {
            printf(INF VK_DBG_PREFIX" Found compatible swapchain surface format\n");
            return formats[i];
        } 
        
    }
    printf(ERR VK_DBG_PREFIX" No suitable surface format for this application, unable to proceed\n");
    exit(1);
}
static VkPresentModeKHR swap_surface_present_mode_choose(VkPresentModeKHR *present_modes, uint32_t present_mode_count)
{
    bool mailbox = false;
    bool fifo = false;

    for (uint32_t i = 0; i < present_mode_count; i++)
    {
        switch (present_modes[i])
        {
            case VK_PRESENT_MODE_FIFO_KHR: fifo = true; break;
            case VK_PRESENT_MODE_MAILBOX_KHR: mailbox = true; break;
        }
    }
    if (!fifo)
    {
        printf(ERR VK_DBG_PREFIX" No appropriate present mode found, unable to proceed\n");
        exit(1);
    }

    if (mailbox) printf(INF VK_DBG_PREFIX" Present mode: mailbox\n");

    return mailbox ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
}
void swapchain_create(VkProcess *process, GLFWwindow *window)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*process->physical_device, *process->surface, &surface_capabilities);

    process->extent_2d = malloc(sizeof(VkExtent2D));
    *process->extent_2d = swap_extent_choose(&surface_capabilities, window);
    uint32_t min_image_count = swap_min_image_count(&surface_capabilities);

    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*process->physical_device, *process->surface, &surface_format_count, NULL);
    VkSurfaceFormatKHR *surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*process->physical_device, *process->surface, &surface_format_count, surface_formats);
    process->surface_format = malloc(sizeof(VkSurfaceFormatKHR));
    *process->surface_format = swap_surface_format_choose(surface_formats, surface_format_count);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*process->physical_device, *process->surface, &present_mode_count, NULL);
    VkPresentModeKHR *present_modes = malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(*process->physical_device, *process->surface, &present_mode_count, present_modes);
    VkPresentModeKHR present_mode = swap_surface_present_mode_choose(present_modes, present_mode_count);

    VkSwapchainCreateInfoKHR swapchain_create_info = (VkSwapchainCreateInfoKHR)
    {
        .surface = *process->surface,
        .minImageCount = min_image_count,
        .imageFormat = process->surface_format->format,
        .imageColorSpace = process->surface_format->colorSpace,
        .imageExtent = *process->extent_2d,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = true,
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
    };

    process->swapchain = malloc(sizeof(VkSwapchainKHR));
    vkCreateSwapchainKHR(*process->logical_device, &swapchain_create_info, NULL, process->swapchain);
    uint32_t swapchain_image_count;
    vkGetSwapchainImagesKHR(*process->logical_device, *process->swapchain, &swapchain_image_count, NULL);
    process->image_count = swapchain_image_count;
    process->images = malloc(sizeof(VkImage) * swapchain_image_count);
    vkGetSwapchainImagesKHR(*process->logical_device, *process->swapchain, &swapchain_image_count, process->images);

    printf(INF VK_DBG_PREFIX" Swapchain created successfully\n");

    free(surface_formats);
    free(present_modes);
}

void image_views_create(VkProcess *process)
{
    VkImageViewCreateInfo image_view_create_info = (VkImageViewCreateInfo)
    {
        .viewType = VK_IMAGE_TYPE_2D,
        .format = process->surface_format->format,
        .subresourceRange = (VkImageSubresourceRange)
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        },
        .components = (VkComponentMapping){},
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
    };

    process->image_view_count = process->image_count;
    process->image_views = malloc(sizeof(VkImageView) * process->image_view_count);
    for (uint32_t i = 0; i < process->image_view_count; i++)
    {
        image_view_create_info.image = process->images[i];
        vkCreateImageView(*process->logical_device, &image_view_create_info, NULL, &process->image_views[i]);
    }
    printf(INF VK_DBG_PREFIX" Loaded %u image view(s)\n", process->image_view_count);
}

void graphics_pipeline_create(VkProcess *process)
{
    const char *shader_file = "slang.spv";
    uint64_t shader_bin_size;
    shader_bin_read(shader_file, &shader_bin_size, NULL);
    uint32_t shader_bin[(shader_bin_size / 4U) + (shader_bin_size % 4U)];
    shader_bin_read(shader_file, &shader_bin_size, shader_bin);

    VkShaderModuleCreateInfo shader_create_info = (VkShaderModuleCreateInfo)
    {
        .codeSize = shader_bin_size,
        .pCode = shader_bin,
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    };

    VkShaderModule shader_module;
    vkCreateShaderModule(*process->logical_device, &shader_create_info, NULL, &shader_module);
    
    VkPipelineShaderStageCreateInfo vertex_shader_create_info = (VkPipelineShaderStageCreateInfo)
    {
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = shader_module,
        .pName = "vertMain",
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    };
    VkPipelineShaderStageCreateInfo fragment_shader_create_info = (VkPipelineShaderStageCreateInfo)
    {
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = shader_module,
        .pName = "fragMain",
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    };

    VkPipelineShaderStageCreateInfo shader_stage_create_infos[] = {vertex_shader_create_info, fragment_shader_create_info};
    printf(INF VK_DBG_PREFIX" Shaders loaded successfully\n");

    

    VkPipelineVertexInputStateCreateInfo vertex_create_info = (VkPipelineVertexInputStateCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = (VkPipelineInputAssemblyStateCreateInfo)
    {
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
    };
    
    VkPipelineViewportStateCreateInfo viewport_state_create_info = (VkPipelineViewportStateCreateInfo)
    {
        .viewportCount = 1,
        .scissorCount = 1,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
    };
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info = (VkPipelineRasterizationStateCreateInfo)
    {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
    };

    VkPipelineMultisampleStateCreateInfo multisamp_create_info = (VkPipelineMultisampleStateCreateInfo)
    {
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info;

    VkPipelineColorBlendAttachmentState color_blend_attachment = (VkPipelineColorBlendAttachmentState)
    {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo color_blend_create_info = (VkPipelineColorBlendStateCreateInfo)
    {
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
    };

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    uint32_t dynamic_state_count;
    ARR_SIZE(&dynamic_state_count, UINT32_MAX, dynamic_states);

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = (VkPipelineDynamicStateCreateInfo)
    {
        .dynamicStateCount = dynamic_state_count,
        .pDynamicStates = dynamic_states,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = (VkPipelineLayoutCreateInfo)
    {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    process->pipeline_layout = malloc(sizeof(VkPipelineLayout));
    vkCreatePipelineLayout(*process->logical_device, &pipeline_layout_create_info, NULL, process->pipeline_layout);
    VkPipelineRenderingCreateInfo pipeline_rendering_create_info = (VkPipelineRenderingCreateInfo)
    {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &process->surface_format->format,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO
    };
    VkGraphicsPipelineCreateInfo pipeline_create_info = (VkGraphicsPipelineCreateInfo)
    {
        .stageCount = 2,
        .pStages = shader_stage_create_infos,
        .pVertexInputState = &vertex_create_info,
        .pInputAssemblyState = &input_assembly_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterizer_create_info,
        .pMultisampleState = &multisamp_create_info,
        .pColorBlendState = &color_blend_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = *process->pipeline_layout,
        .renderPass = NULL,
        .pNext = &pipeline_rendering_create_info,
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
    };
    
    process->graphics_pipeline = malloc(sizeof(VkPipeline));
    vkCreateGraphicsPipelines(*process->logical_device, NULL, 1, &pipeline_create_info, NULL, process->graphics_pipeline);
    printf(INF VK_DBG_PREFIX" Graphics pipeline created successfully\n");
}
void command_pool_create(VkProcess *process)
{
    VkCommandPoolCreateInfo pool_create_info = (VkCommandPoolCreateInfo)
    {
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = process->graphics_queue_index,
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
    };
    process->command_pool = malloc(sizeof(VkCommandPool));
    vkCreateCommandPool(*process->logical_device, &pool_create_info, NULL, process->command_pool);
    printf(INF VK_DBG_PREFIX" Command pool created successfully\n");
}
void command_buffer_create(VkProcess *process)
{
    process->command_buffer_count = 1;
    VkCommandBufferAllocateInfo alloc_info = (VkCommandBufferAllocateInfo)
    {
        .commandPool = *process->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = process->command_buffer_count,
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
    };
    process->command_buffers = malloc(sizeof(VkCommandBuffer) * process->command_buffer_count);
    vkAllocateCommandBuffers(*process->logical_device, &alloc_info, process->command_buffers);
    printf(INF VK_DBG_PREFIX" Allocated 0x%p for command buffer, size: %u byte(s)\n", (void *)process->command_buffers, sizeof(VkCommandBuffer) * process->command_buffer_count);
}
static void transition_image_layout
(
    VkProcess *proc,
    uint32_t image_index,
    VkImageLayout layout_old,
    VkImageLayout layout_new,
    VkAccessFlags2 src_access_mask,
    VkAccessFlags2 dst_access_mask,
    VkPipelineStageFlags2 src_stage_mask,
    VkPipelineStageFlags2 dst_stage_mask
)
{
    VkImageMemoryBarrier2 barrier = (VkImageMemoryBarrier2)
    {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = layout_old,
        .newLayout = layout_new,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = proc->images[image_index],
        .subresourceRange = (VkImageSubresourceRange)
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2
    };
    VkDependencyInfo dependency_info = (VkDependencyInfo)
    {
        .dependencyFlags = 0,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO
    };
    vkCmdPipelineBarrier2(proc->command_buffers[0], &dependency_info);
}
void command_buffer_record(VkProcess *process, uint32_t image_index)
{
    VkCommandBufferBeginInfo buffer_begin_info = (VkCommandBufferBeginInfo)
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    vkBeginCommandBuffer(process->command_buffers[0], &buffer_begin_info);

    transition_image_layout
    (
        process,
        image_index,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        0,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    VkClearValue clear_color = (VkClearValue)
    {
        .color = (VkClearColorValue)
        {
            .float32 = {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };
    VkRenderingAttachmentInfo attachement_info = (VkRenderingAttachmentInfo)
    {
        .imageView = process->image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_color,
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO
    };
    VkRenderingInfo render_info = (VkRenderingInfo)
    {
        .renderArea = (VkRect2D)
        {
            .offset = (VkOffset2D)
            {
                0,
                0
            },
            .extent = *process->extent_2d
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachement_info,
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO
    };

    VkViewport viewport = (VkViewport)
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (*process->extent_2d).width,
        .height = (*process->extent_2d).height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = (VkRect2D)
    {
        .offset = (VkOffset2D)
        {
            .x = 0,
            .y = 0
        },
        .extent = *process->extent_2d
    };
    vkCmdBeginRendering(process->command_buffers[0], &render_info);
    vkCmdBindPipeline(process->command_buffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, *process->graphics_pipeline);
    vkCmdSetViewport(process->command_buffers[0], 0, 1, &viewport);
    vkCmdSetScissor(process->command_buffers[0], 0, 1, &scissor);
    vkCmdDraw(process->command_buffers[0], 3, 1, 0, 0);
    vkCmdEndRendering(process->command_buffers[0]);
    transition_image_layout
    (
        process,
        image_index,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    );
    vkEndCommandBuffer(process->command_buffers[0]);
}

void sync_object_create(VkProcess *process)
{
    process->present_complete_semaphore = malloc(sizeof(VkSemaphore));
    VkSemaphoreCreateInfo present_sema_create_info = (VkSemaphoreCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    vkCreateSemaphore(*process->logical_device, &present_sema_create_info, NULL, process->present_complete_semaphore);

    process->render_finished_semaphore = malloc(sizeof(VkSemaphore));
    VkSemaphoreCreateInfo render_sema_create_info = (VkSemaphoreCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    vkCreateSemaphore(*process->logical_device, &render_sema_create_info, NULL, process->render_finished_semaphore);

    process->fence = malloc(sizeof(VkFence));
    VkFenceCreateInfo fence_create_info = (VkFenceCreateInfo)
    {
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    vkCreateFence(*process->logical_device, &fence_create_info, NULL, process->fence);
}
void frame_draw(VkProcess *process)
{
    if (vkWaitForFences(*process->logical_device, 1U, process->fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
    {
        printf(ERR VK_DBG_PREFIX" Failed to wait for fence, unable to proceed\n");
        exit(1);
    }
    vkResetFences(*process->logical_device, 1U, process->fence);

    VkResult result;
    uint32_t image_index;
    result = vkAcquireNextImageKHR
    (
        *process->logical_device,
        *process->swapchain,
        UINT64_MAX,
        *process->present_complete_semaphore,
        NULL,
        &image_index
    );
    command_buffer_record(process, image_index);

    vkQueueWaitIdle(*process->graphics_queue);

    VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = (VkSubmitInfo)
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = process->present_complete_semaphore,
        .pWaitDstStageMask = &stage_mask,
        .commandBufferCount = process->command_buffer_count,
        .pCommandBuffers = process->command_buffers,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = process->render_finished_semaphore,
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };
    vkQueueSubmit(*process->graphics_queue, 1, &submit_info, *process->fence);

    VkPresentInfoKHR present_info = (VkPresentInfoKHR)
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = process->render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = process->swapchain,
        .pImageIndices = &image_index,
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
    };
    result = vkQueuePresentKHR(*process->graphics_queue, &present_info);
    switch (result)
    {
        case VK_SUCCESS: break;
        case VK_SUBOPTIMAL_KHR: printf(WARN VK_DBG_PREFIX" vkQueuePresentKHR returns VK_SUBOPTIMAL_KHR\n"); break;
        default: printf(WARN VK_DBG_PREFIX" Unexpected result returned from vkQueuePresentKHR\n");
    }
 
}
#endif