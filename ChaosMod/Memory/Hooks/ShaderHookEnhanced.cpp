#include <stdafx.h>

#include "Memory/Hooks/Hook.h"
#include "ShaderHookEnhanced.h"

#include "Components/SplashTexts.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Util/DX12.h"

#include "ShaderHook.h"

#if 0
#define SHADER_LOG DEBUG_LOG
#else
#define SHADER_LOG(...)
#endif

static void **ms_PresentAddr               = nullptr;
static void **ms_ExecuteCommandListsAddr   = nullptr;
static void **ms_DrawInstancedAddr         = nullptr;
static void **ms_OMSetRenderTargetsAddr    = nullptr;
static void **ms_ResourceBarrierAddr       = nullptr;
static void **ms_ResetAddr                 = nullptr;
static ID3D12Device7 *ms_Device            = nullptr;
static ID3D12CommandQueue *ms_CommandQueue = nullptr;

static std::vector<ID3D12Resource *> ms_BackBuffers {};

static bool ms_InjectRecurseCheck = false;

struct InjectInfo
{
	bool m_Active;
	ID3D12CommandList *m_ListToInject;
	ID3D12Resource *m_Resource;
	D3D12_RESOURCE_STATES m_State;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDesc;
};

InjectInfo ms_BackBufferInjectInfo;
InjectInfo ms_DepthBufferInjectInfo;

struct CommandListInfo
{
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> depthTargets;
};

static std::map<ID3D12CommandList *, CommandListInfo> ms_CommandListInfos;

static std::mutex ms_BackBufferVectorMutex;
static std::mutex ms_CommandListMapMutex;
static std::mutex ms_ShaderVectorMutex;

struct ShaderInfo
{
	std::string src;
	bool isLegacy;
};

static std::vector<ShaderInfo> ms_Shaders;

static HRESULT (*OG_IDXGISwapChain_Present)(IDXGISwapChain *, UINT, UINT);
static HRESULT HK_IDXGISwapChain_Present(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags)
{
	DXGI_SWAP_CHAIN_DESC desc;
	swapChain->GetDesc(&desc);

	SHADER_LOG("Present hook; buffer count=" << desc.BufferCount << " flags=" << desc.Flags);

	{
		std::lock_guard lock(ms_BackBufferVectorMutex);
		ms_BackBuffers.clear();
		for (size_t i = 0; i < desc.BufferCount; i++)
		{
			ComPointer<ID3D12Resource> backBuffer = nullptr;
			swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			// SaveImage(backBuffer, i);
			ms_BackBuffers.push_back(backBuffer);
		}
	}

	if (!DX12PipelineInjector::Get().IsInit())
	{
		if (desc.BufferCount > 0)
		{
			SHADER_LOG("Initting injector");
			ComPointer<ID3D12Resource> backBuffer;
			swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
			D3D12_RESOURCE_DESC bbDesc = backBuffer->GetDesc();
			DX12PipelineInjector::Get().SetDimensions(bbDesc.Width, bbDesc.Height);
			DX12PipelineInjector::Get().Init();
			SHADER_LOG("Initted injector");
		}
	}

	return OG_IDXGISwapChain_Present(swapChain, syncInterval, flags);
}

static void (*OG_DrawInstanced)(ID3D12GraphicsCommandList *_this, UINT VertexCountPerInstance, UINT InstanceCount,
                                UINT StartVertexLocation, UINT StartInstanceLocation);
static void HK_DrawInstanced(ID3D12GraphicsCommandList *_this, UINT VertexCountPerInstance, UINT InstanceCount,
                             UINT StartVertexLocation, UINT StartInstanceLocation)
{
	SHADER_LOG("DrawInstanced hook " << _this << " " << VertexCountPerInstance << " " << InstanceCount << " "
	                                 << StartVertexLocation << " " << StartInstanceLocation);

	OG_DrawInstanced(_this, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);

	if (ms_BackBufferInjectInfo.m_Active && _this == ms_BackBufferInjectInfo.m_ListToInject)
	{
		ms_BackBufferInjectInfo.m_Active = false;

		if (DX12PipelineInjector::Get().IsInit())
		{
			ms_InjectRecurseCheck = true;
			std::lock_guard lock(ms_ShaderVectorMutex);
			for (const auto shader : ms_Shaders)
			{
				DX12PipelineInjector::ResourceInfo backBufferInfo {
					.Resource = ms_BackBufferInjectInfo.m_Resource,
					.View = ms_BackBufferInjectInfo.m_CPUDesc,
					.State = ms_BackBufferInjectInfo.m_State
				};
				DX12PipelineInjector::ResourceInfo depthBufferInfo {
					.Resource = ms_DepthBufferInjectInfo.m_Resource,
					.View = ms_DepthBufferInjectInfo.m_CPUDesc,
					.State = ms_DepthBufferInjectInfo.m_State
				};
				DX12PipelineInjector::Get().InjectPixelShader(_this, backBufferInfo, depthBufferInfo, shader.src);
			}
			ms_InjectRecurseCheck = false;
		}
	}
}

static void (*OG_ResourceBarrier)(ID3D12GraphicsCommandList *_this, UINT NumBarriers,
                                  const D3D12_RESOURCE_BARRIER *pBarriers);
static void HK_ResourceBarrier(ID3D12GraphicsCommandList *_this, UINT NumBarriers,
                               const D3D12_RESOURCE_BARRIER *pBarriers)
{

	SHADER_LOG("ResourceBarrier hook " << _this << " " << NumBarriers);
	OG_ResourceBarrier(_this, NumBarriers, pBarriers);
	if (ms_InjectRecurseCheck)
		return;

	for (size_t i = 0; i < NumBarriers; i++)
	{
		SHADER_LOG("Barrier " << i << " " << pBarriers[i].Flags << " " << pBarriers[i].Type);

		const auto barrier = pBarriers[i];

		std::lock_guard lock(ms_BackBufferVectorMutex);
		std::lock_guard lock1(ms_CommandListMapMutex);
		for (size_t j = 0; j < ms_BackBuffers.size(); j++)
		{
			ID3D12Resource *backBuffer = ms_BackBuffers[j];
			if (backBuffer == barrier.Transition.pResource)
			{
				SHADER_LOG("FOUND BACK BUFFER: " << backBuffer << " (" << j << ")");
				if (barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE
				    && ms_CommandListInfos[_this].renderTargets.size() > 0)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE desc       = ms_CommandListInfos[_this].renderTargets.back();

					ms_BackBufferInjectInfo.m_Active       = true;
					ms_BackBufferInjectInfo.m_Resource     = backBuffer;
					ms_BackBufferInjectInfo.m_ListToInject = _this;
					ms_BackBufferInjectInfo.m_CPUDesc      = desc;
					ms_BackBufferInjectInfo.m_State        = barrier.Transition.StateAfter;
				}
			}
		}
	}

	if (NumBarriers == 1)
	{
		const auto barrier = pBarriers[0];
		if (barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_COPY_SOURCE
		    && barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_DEPTH_WRITE)
		{
			if (ms_CommandListInfos[_this].depthTargets.size() > 0)
			{
				SHADER_LOG("FOUND DEPTH BUFFER: " << barrier.Transition.pResource);

				D3D12_CPU_DESCRIPTOR_HANDLE desc        = ms_CommandListInfos[_this].depthTargets.back();

				ms_DepthBufferInjectInfo.m_Active       = true;
				ms_DepthBufferInjectInfo.m_Resource     = barrier.Transition.pResource;
				ms_DepthBufferInjectInfo.m_ListToInject = _this;
				ms_DepthBufferInjectInfo.m_CPUDesc      = desc;
				ms_DepthBufferInjectInfo.m_State        = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			}
		}
	}
}

static void (*OG_SetDescriptorHeaps)(ID3D12GraphicsCommandList *_this, UINT NumDescriptorHeaps,
                                     ID3D12DescriptorHeap **ppDescriptorHeaps);
static void HK_SetDescriptorHeaps(ID3D12GraphicsCommandList *_this, UINT NumDescriptorHeaps,
                                  ID3D12DescriptorHeap **ppDescriptorHeaps)
{
	SHADER_LOG("SetDescriptorHeaps hook " << _this);
	OG_SetDescriptorHeaps(_this, NumDescriptorHeaps, ppDescriptorHeaps);
}

static void (*OG_Reset)(ID3D12GraphicsCommandList *_this, ID3D12CommandAllocator *pAllocator,
                        ID3D12PipelineState *pInitialState);
static void HK_Reset(ID3D12GraphicsCommandList *_this, ID3D12CommandAllocator *pAllocator,
                     ID3D12PipelineState *pInitialState)
{
	SHADER_LOG("Reset hook " << _this);
	if (!ms_InjectRecurseCheck)
	{
		std::lock_guard lock(ms_CommandListMapMutex);
		ms_CommandListInfos.erase(_this);
	}
	OG_Reset(_this, pAllocator, pInitialState);
}

static void (*OG_OMSetRenderTargets)(ID3D12GraphicsCommandList *_this, UINT NumRenderTargetDescriptors,
                                     D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
                                     BOOL RTsSingleHandleToDescriptorRange,
                                     D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor);
static void HK_OMSetRenderTargets(ID3D12GraphicsCommandList *_this, UINT NumRenderTargetDescriptors,
                                  D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
                                  BOOL RTsSingleHandleToDescriptorRange,
                                  D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor)
{
	SHADER_LOG("OMSetRenderTargets hook " << _this << " " << NumRenderTargetDescriptors << " "
	                                      << pRenderTargetDescriptors << " " << RTsSingleHandleToDescriptorRange << " "
	                                      << pDepthStencilDescriptor);

	if (!ms_InjectRecurseCheck)
	{
		std::lock_guard lock(ms_CommandListMapMutex);
		for (size_t i = 0; i < NumRenderTargetDescriptors; i++)
			ms_CommandListInfos[_this].renderTargets.push_back(pRenderTargetDescriptors[i]);
		if (pDepthStencilDescriptor)
			ms_CommandListInfos[_this].depthTargets.push_back(*pDepthStencilDescriptor);
	}

	OG_OMSetRenderTargets(_this, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange,
	                      pDepthStencilDescriptor);
}

static void HookCommandListFunctions(ID3D12CommandList *commandList)
{
	SHADER_LOG("Hooking GraphicsCommandList functions; commandList=" << commandList << " vft="
	                                                                 << *reinterpret_cast<DWORD64 *>(commandList));

	Handle handle = *reinterpret_cast<DWORD64 *>(commandList);

	if (!ms_DrawInstancedAddr)
	{
		ms_DrawInstancedAddr = handle.At(0x60).Get<void *>();

		OG_DrawInstanced     = *(void (**)(ID3D12GraphicsCommandList *, UINT, UINT, UINT, UINT))ms_DrawInstancedAddr;
		SHADER_LOG("ms_DrawInstancedAddr=" << ms_DrawInstancedAddr << " OG_DrawInstanced=" << OG_DrawInstanced);

		Memory::Write<void *>(ms_DrawInstancedAddr, reinterpret_cast<void *>(HK_DrawInstanced));
	}

	if (!ms_OMSetRenderTargetsAddr)
	{
		ms_OMSetRenderTargetsAddr = handle.At(0x170).Get<void *>();
		OG_OMSetRenderTargets     = *(void (**)(ID3D12GraphicsCommandList *, UINT, D3D12_CPU_DESCRIPTOR_HANDLE *, BOOL,
                                            D3D12_CPU_DESCRIPTOR_HANDLE *))ms_OMSetRenderTargetsAddr;

		Memory::Write<void *>(ms_OMSetRenderTargetsAddr, reinterpret_cast<void *>(HK_OMSetRenderTargets));
	}

	if (!ms_ResourceBarrierAddr)
	{
		ms_ResourceBarrierAddr = handle.At(0xD0).Get<void *>();
		OG_ResourceBarrier     = *(void (**)(ID3D12GraphicsCommandList *_this, UINT NumBarriers,
                                         const D3D12_RESOURCE_BARRIER *))ms_ResourceBarrierAddr;
		SHADER_LOG("ms_ResourceBarrierAddr=" << ms_ResourceBarrierAddr << " OG_ResourceBarrier=" << OG_ResourceBarrier);

		Memory::Write<void *>(ms_ResourceBarrierAddr, reinterpret_cast<void *>(HK_ResourceBarrier));
	}

	if (!ms_ResetAddr)
	{
		ms_ResetAddr = handle.At(0x50).Get<void *>();
		OG_Reset     = *(void (**)(ID3D12GraphicsCommandList *_this, ID3D12CommandAllocator *pAllocator,
                               ID3D12PipelineState *pInitialState))ms_ResetAddr;

		Memory::Write<void *>(ms_ResetAddr, reinterpret_cast<void *>(HK_Reset));
	}
}

static HRESULT (*OG_ExecuteCommandLists)(ID3D12CommandQueue *_this, UINT NumCommandLists,
                                         ID3D12CommandList **ppCommandLists);
static HRESULT HK_ExecuteCommandLists(ID3D12CommandQueue *_this, UINT NumCommandLists,
                                      ID3D12CommandList **ppCommandLists)
{
	SHADER_LOG("ExecuteCommandLists hook; this=" << _this << " NumCommandLists " << NumCommandLists);

	auto res = OG_ExecuteCommandLists(_this, NumCommandLists, ppCommandLists);

	for (size_t i = 0; i < NumCommandLists; i++)
	{
		SHADER_LOG("CommandList " << i << ": " << ppCommandLists[i] << " type " << ppCommandLists[i]->GetType()
		                          << " vtable " << *reinterpret_cast<DWORD64 *>(ppCommandLists[i]));

		if (ppCommandLists[i]->GetType() == 0)
			HookCommandListFunctions(ppCommandLists[i]);
	}

	ms_CommandQueue = _this;

	return res;
}

static bool OnHook()
{
	if (!IsEnhanced())
		return false;

	ms_PresentAddr = ms_ExecuteCommandListsAddr = ms_DrawInstancedAddr = ms_ResourceBarrierAddr = ms_ResetAddr =
	    nullptr;

	ms_Device             = nullptr;
	ms_CommandQueue       = nullptr;

	ms_InjectRecurseCheck = false;

	ms_BackBuffers.clear();
	ms_CommandListInfos.clear();
	ms_Shaders.clear();

	Handle handle;

	handle = Memory::FindPattern("4c 8b 05 ? ? ? ? 48 8b 15 ? ? ? ? 48 83 ec");
	if (!handle.IsValid())
		return false;

	// IDXGISwapChain
	handle                    = *handle.At(2).Into().Value<DWORD64 *>();

	ms_PresentAddr            = handle.At(0x40).Get<void *>();
	OG_IDXGISwapChain_Present = *(HRESULT(**)(IDXGISwapChain *, UINT, UINT))ms_PresentAddr;
	Memory::Write<void *>(ms_PresentAddr, reinterpret_cast<void *>(HK_IDXGISwapChain_Present));

	handle = Memory::FindPattern("48 8B 0D ?? ?? ?? ?? C1 E0 0A");
	if (!handle.IsValid())
		return false;

	handle    = handle.At(2).Into();
	ms_Device = handle.Value<ID3D12Device7 *>();

	handle    = *handle.Value<DWORD64 *>();

	handle    = Memory::FindPattern("48 8B 0D ?? ?? ?? ?? 48 8B 01 BA 01 00 00 00 4D 89 F8");
	if (!handle.IsValid())
		return false;

	// ID3D12CommandQueue vtable
	handle                     = *handle.At(2).Into().Value<DWORD64 *>();
	ms_ExecuteCommandListsAddr = handle.At(0x50).Get<void *>();
	OG_ExecuteCommandLists = *(HRESULT(**)(ID3D12CommandQueue *, UINT, ID3D12CommandList **))ms_ExecuteCommandListsAddr;
	Memory::Write<void *>(ms_ExecuteCommandListsAddr, reinterpret_cast<void *>(HK_ExecuteCommandLists));

	DX12PipelineInjector::Get().SetDevice(ms_Device);

	return true;
}

static void OnCleanup()
{
	if (!IsEnhanced())
		return;

	SHADER_LOG("Cleanup");

	{
		std::lock_guard lock1(ms_BackBufferVectorMutex);
		std::lock_guard lock2(ms_CommandListMapMutex);
		std::lock_guard lock3(ms_ShaderVectorMutex);

		ms_BackBuffers.clear();
		ms_CommandListInfos.clear();
		ms_Shaders.clear();
	}

	SHADER_LOG("Cleared command maps");

	// Only reset vftable entries if address still points to our retour
	if (ms_PresentAddr && *ms_PresentAddr == HK_IDXGISwapChain_Present)
		Memory::Write<void *>(ms_PresentAddr, reinterpret_cast<void *>(OG_IDXGISwapChain_Present));

	if (ms_ExecuteCommandListsAddr && *ms_ExecuteCommandListsAddr == HK_ExecuteCommandLists)
		Memory::Write<void *>(ms_ExecuteCommandListsAddr, reinterpret_cast<void *>(OG_ExecuteCommandLists));

	if (ms_DrawInstancedAddr && *ms_DrawInstancedAddr == HK_DrawInstanced)
		Memory::Write<void *>(ms_DrawInstancedAddr, reinterpret_cast<void *>(OG_DrawInstanced));

	if (ms_OMSetRenderTargetsAddr && *ms_OMSetRenderTargetsAddr == HK_OMSetRenderTargets)
		Memory::Write<void *>(ms_OMSetRenderTargetsAddr, reinterpret_cast<void *>(OG_OMSetRenderTargets));

	if (ms_ResourceBarrierAddr && *ms_ResourceBarrierAddr == HK_ResourceBarrier)
		Memory::Write<void *>(ms_ResourceBarrierAddr, reinterpret_cast<void *>(OG_ResourceBarrier));

	if (ms_ResetAddr && *ms_ResetAddr == HK_Reset)
		Memory::Write<void *>(ms_ResetAddr, reinterpret_cast<void *>(OG_Reset));

	SHADER_LOG("Cleaned up hooks");

	DX12PipelineInjector::Get().UnInit();

	SHADER_LOG("Uninit injector");
}

static RegisterHook registerHook(OnHook, OnCleanup, "EnhancedShaderHook", true);

namespace Hooks
{
	void AddPixelShader(std::string_view shaderSrc, bool isLegacy)
	{
		std::lock_guard lock(ms_ShaderVectorMutex);
		ms_Shaders.emplace_back(std::string(shaderSrc), isLegacy);
	}

	void AddPixelShader(std::string_view shaderSrc)
	{
		AddPixelShader(shaderSrc, false);
	}

	void RemovePixelShader(std::string_view shaderSrc)
	{
		std::lock_guard lock(ms_ShaderVectorMutex);
		for (auto it = ms_Shaders.begin(); it != ms_Shaders.end();)
			if (shaderSrc.compare(it->src))
				it++;
			else
				it = ms_Shaders.erase(it);
	}
}