#include "ComPointer.hpp"

#include "d3d12.h"
#include "d3dcompiler.h"
#include "dxgi1_6.h"

#include "string_view"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

class DX12PipelineInjector
{
  public:
	struct ResourceInfo
	{
		ID3D12Resource *Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE View;
		D3D12_RESOURCE_STATES State;
	};

  private:
	static const char *ms_DefaultPixelShader;
	static const char *ms_DefaultVertexShader;

	std::unordered_map<size_t, std::vector<BYTE>> shaderCache;

	bool m_Init;

	int m_Width;
	int m_Height;

	ID3D12Device7 *m_Device;

	ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
	ComPointer<ID3D12DescriptorHeap> m_RtvHeap;
	ComPointer<ID3D12Resource> m_SceneTexture;
	ComPointer<ID3D12Resource> m_DepthBuffer;
	ComPointer<ID3D12RootSignature> m_PostProcessRootSignature;
	ComPointer<ID3D12Resource> m_VertexBuffer;
	ComPointer<ID3D12PipelineState> m_PostProcessPSO;
	D3D12_INPUT_ELEMENT_DESC m_InputLayout[2];

	std::unordered_map<size_t, ComPointer<ID3D12PipelineState>> m_PSOMap;

  public:
	void InjectShaders(ID3D12GraphicsCommandList *commandList, ResourceInfo backBufferInfo,
	                   ResourceInfo depthBufferInfo, std::string_view pixelShader, std::string_view vertexShader);
	inline void InjectShaders(ID3D12GraphicsCommandList *commandList, ResourceInfo backBufferInfo,
	                          ResourceInfo depthBufferInfo)
	{
		InjectShaders(commandList, backBufferInfo, depthBufferInfo, ms_DefaultPixelShader, ms_DefaultVertexShader);
	}
	inline void InjectPixelShader(ID3D12GraphicsCommandList *commandList, ResourceInfo backBufferInfo,
	                              ResourceInfo depthBufferInfo, std::string_view shaderSrc)
	{
		InjectShaders(commandList, backBufferInfo, depthBufferInfo, shaderSrc, ms_DefaultVertexShader);
	}
	inline void InjectVertexShader(ID3D12GraphicsCommandList *commandList, ResourceInfo backBufferInfo,
	                               ResourceInfo depthBufferInfo, std::string_view shaderSrc)
	{
		InjectShaders(commandList, backBufferInfo, depthBufferInfo, ms_DefaultPixelShader, shaderSrc);
	}

	inline void SetDevice(ID3D12Device7 *device)
	{
		m_Device = device;
	}

	inline void SetDimensions(int width, int height)
	{
		m_Width  = width;
		m_Height = height;
	}

	inline bool CanInit()
	{
		return m_Device && m_Width && m_Height;
	}

	inline void Init()
	{
		if (CanInit() && !IsInit())
		{
			CreateVertexBuffer();
			CreateTexture();
			CreatePostProcessRootSignature();
			m_Init = true;
		}
	}

	inline bool IsInit()
	{
		return m_Init;
	}

	inline void UnInit()
	{
		m_Device = nullptr;
		m_Width = m_Height = 0;
		m_PSOMap.clear();

		m_Init = false;
	}

  private:
	void CreatePostProcessRootSignature();
	void CreateVertexBuffer();
	void CreatePostProcessPSO(std::string_view pixelShader, std::string_view vertexShader,
	                          ID3D12PipelineState **result);
	void CreateTexture();

	inline size_t GetHash(std::string_view pixelShader, std::string_view vertexShader)
	{
		static const auto hash = std::hash<std::string_view>();
		return hash(pixelShader) * 17 + hash(vertexShader);
	}

	// singleton
  public:
	DX12PipelineInjector(const DX12PipelineInjector &)            = delete;
	DX12PipelineInjector &operator=(const DX12PipelineInjector &) = delete;

	inline static DX12PipelineInjector &Get()
	{
		static DX12PipelineInjector instance;
		return instance;
	}

  private:
	DX12PipelineInjector() = default;
};

inline const char *DX12PipelineInjector::ms_DefaultPixelShader  = R"---(
Texture2D inputTex : register(t0);
SamplerState textureSampler : register(s0);

void main(
    // == IN ==
    in float2 uv : Texcoord,

    // == OUT ==
    out float4 pixel : SV_Target
)
{
    float4 texel = inputTex.Sample(textureSampler, uv);
    pixel = float4(texel.rgb, 1.0f);
}
)---";
inline const char *DX12PipelineInjector::ms_DefaultVertexShader = R"---(
void main(
    // == IN ==
    in float2 pos : Position,
    in float2 uv : Texcoord,

    // == OUT ==
    out float2 o_uv : Texcoord,
    out float4 o_pos : SV_Position
)
{
    o_pos = float4(pos, 1.0f, 1.0f);
    o_uv = uv;
}
)---";