#include "DX12.h"

#include "stdafx.h"

struct Vertex
{
	float x, y, u, v;
};

static void AddBarrier(ID3D12GraphicsCommandList *commandList, ID3D12Resource *resource,
                       D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	D3D12_RESOURCE_BARRIER barr;
	barr.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barr.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barr.Transition.pResource   = resource;
	barr.Transition.Subresource = 0;
	barr.Transition.StateBefore = beforeState;
	barr.Transition.StateAfter  = afterState;

	commandList->ResourceBarrier(1, &barr);
}

void DX12PipelineInjector::CreatePostProcessRootSignature()
{
	HRESULT hr                                               = S_OK;
	D3D12_DESCRIPTOR_RANGE srvTable                          = {};
	srvTable.RangeType                                       = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvTable.NumDescriptors                                  = 1;
	srvTable.BaseShaderRegister                              = 0;
	srvTable.RegisterSpace                                   = 0;
	srvTable.OffsetInDescriptorsFromTableStart               = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER slotRootParameter[2]                = {};

	slotRootParameter[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	slotRootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
	slotRootParameter[0].DescriptorTable.pDescriptorRanges   = &srvTable;
	slotRootParameter[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

	slotRootParameter[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	slotRootParameter[1].Constants.ShaderRegister            = 0;
	slotRootParameter[1].Constants.RegisterSpace             = 0;
	slotRootParameter[1].Constants.Num32BitValues            = 2;
	slotRootParameter[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC samplerDesc                    = {};
	samplerDesc.Filter                                       = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU                                     = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV                                     = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW                                     = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias                                   = 0.0f;
	samplerDesc.MaxAnisotropy                                = 16;
	samplerDesc.ComparisonFunc                               = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.BorderColor                                  = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.MinLOD                                       = 0.0f;
	samplerDesc.MaxLOD                                       = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister                               = 0;
	samplerDesc.RegisterSpace                                = 0;
	samplerDesc.ShaderVisibility                             = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc                         = {};
	rsDesc.NumParameters                                     = 2;
	rsDesc.pParameters                                       = slotRootParameter;
	rsDesc.NumStaticSamplers                                 = 1;
	rsDesc.pStaticSamplers                                   = &samplerDesc;
	rsDesc.Flags                               = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPointer<ID3DBlob> serializedRootSigBlob = NULL;
	ComPointer<ID3DBlob> errorBlob             = NULL;

	hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSigBlob, &errorBlob);

	hr = m_Device->CreateRootSignature(0, serializedRootSigBlob->GetBufferPointer(),
	                                   serializedRootSigBlob->GetBufferSize(), __uuidof(ID3D12RootSignature),
	                                   reinterpret_cast<void **>(&m_PostProcessRootSignature));

	LOG("Created rootsig: " << m_PostProcessRootSignature.Get());
}

void DX12PipelineInjector::CreateVertexBuffer()
{
	LOG("Creating vertex buffer");

	Vertex rectangle[] = {
		{ -1.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, -1.0f, 1.0f, 1.0f }, { -1.0f, -1.0f, 0.0f, 1.0f },

		{ -1.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.0f },  { 1.0f, -1.0f, 1.0f, 1.0f },
	};

	const UINT vertexBufferSize     = sizeof(rectangle);

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type                  = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask      = 1;
	heapProps.VisibleNodeMask       = 1;

	D3D12_RESOURCE_DESC bufferDesc  = {};
	bufferDesc.Dimension            = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment            = 0;
	bufferDesc.Width                = vertexBufferSize;
	bufferDesc.Height               = 1;
	bufferDesc.DepthOrArraySize     = 1;
	bufferDesc.MipLevels            = 1;
	bufferDesc.Format               = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count     = 1;
	bufferDesc.SampleDesc.Quality   = 0;
	bufferDesc.Layout               = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags                = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr =
	    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
	                                      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBuffer));

	if (FAILED(hr))
		LOG("Failed to create vertex buffer! Shader effects will not work and the game may or may not crash");

	UINT8 *pVertexDataBegin = nullptr;
	D3D12_RANGE readRange   = { 0, 0 }; // We do not intend to read from this resource on the CPU.
	hr                      = m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin));
	if (SUCCEEDED(hr))
	{
		memcpy(pVertexDataBegin, rectangle, sizeof(rectangle));
		m_VertexBuffer->Unmap(0, nullptr);
	}

	LOG("Created vertex buffer: " << m_VertexBuffer.Get());
}

void DX12PipelineInjector::CreateTexture()
{
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment           = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	texDesc.Width               = m_Width;
	texDesc.Height              = m_Height;
	texDesc.DepthOrArraySize    = 1;
	texDesc.MipLevels           = 1;
	texDesc.Format              = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count    = 1;
	texDesc.SampleDesc.Quality  = 0;
	texDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type                  = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask      = 0;
	heapProps.VisibleNodeMask       = 0;

	hr = m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COMMON,
	                                       nullptr, IID_PPV_ARGS(&m_SceneTexture));

	LOG("Scene texture " << m_SceneTexture.Get() << " " << m_Width << " " << m_Height);

	// Create SRV

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NodeMask       = 0;
	heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr                      = m_Device->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap),
	                                                         reinterpret_cast<void **>(&m_SRVHeap));

	LOG("Created descriptor heap: " << m_SRVHeap.Get());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                          = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels             = 1;
	m_Device->CreateShaderResourceView(m_SceneTexture.Get(), &srvDesc, m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
}

void DX12PipelineInjector::CreatePostProcessPSO(std::string_view pixelShader, std::string_view vertexShader,
                                                ID3D12PipelineState **result)
{
	HRESULT hr;

	ComPointer<ID3DBlob> VSByteCode;
	ComPointer<ID3DBlob> PSByteCode;

	hr = D3DCompile(vertexShader.data(), vertexShader.size(), NULL, NULL, NULL, "main", "vs_5_0", 0, 0, &VSByteCode,
	                NULL);

	if (FAILED(hr))
		return;

	hr =
	    D3DCompile(pixelShader.data(), pixelShader.size(), NULL, NULL, NULL, "main", "ps_5_0", 0, 0, &PSByteCode, NULL);

	if (FAILED(hr))
		return;

	m_InputLayout[0] = D3D12_INPUT_ELEMENT_DESC(
	    { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	m_InputLayout[1] = D3D12_INPUT_ELEMENT_DESC({ "Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2,
	                                              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod {};
	gfxPsod.pRootSignature                                   = m_PostProcessRootSignature.Get();
	gfxPsod.InputLayout.NumElements                          = _countof(m_InputLayout);
	gfxPsod.InputLayout.pInputElementDescs                   = m_InputLayout;
	gfxPsod.IBStripCutValue                                  = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gfxPsod.VS.BytecodeLength                                = VSByteCode->GetBufferSize();
	gfxPsod.VS.pShaderBytecode                               = VSByteCode->GetBufferPointer();
	gfxPsod.PS.BytecodeLength                                = PSByteCode->GetBufferSize();
	gfxPsod.PS.pShaderBytecode                               = PSByteCode->GetBufferPointer();
	gfxPsod.DS.BytecodeLength                                = 0;
	gfxPsod.DS.pShaderBytecode                               = nullptr;
	gfxPsod.HS.BytecodeLength                                = 0;
	gfxPsod.HS.pShaderBytecode                               = nullptr;
	gfxPsod.GS.BytecodeLength                                = 0;
	gfxPsod.GS.pShaderBytecode                               = nullptr;
	gfxPsod.PrimitiveTopologyType                            = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gfxPsod.RasterizerState.FillMode                         = D3D12_FILL_MODE_SOLID;
	gfxPsod.RasterizerState.CullMode                         = D3D12_CULL_MODE_NONE;
	gfxPsod.RasterizerState.FrontCounterClockwise            = FALSE;
	gfxPsod.RasterizerState.DepthBias                        = 0;
	gfxPsod.RasterizerState.DepthBiasClamp                   = .0f;
	gfxPsod.RasterizerState.SlopeScaledDepthBias             = .0f;
	gfxPsod.RasterizerState.DepthClipEnable                  = FALSE;
	gfxPsod.RasterizerState.MultisampleEnable                = FALSE;
	gfxPsod.RasterizerState.AntialiasedLineEnable            = FALSE;
	gfxPsod.RasterizerState.ForcedSampleCount                = 0;
	gfxPsod.StreamOutput.NumEntries                          = 0;
	gfxPsod.StreamOutput.NumStrides                          = 0;
	gfxPsod.StreamOutput.pBufferStrides                      = nullptr;
	gfxPsod.StreamOutput.pSODeclaration                      = nullptr;
	gfxPsod.StreamOutput.RasterizedStream                    = 0;
	gfxPsod.NumRenderTargets                                 = 1;
	gfxPsod.RTVFormats[0]                                    = DXGI_FORMAT_B8G8R8A8_UNORM;
	gfxPsod.DSVFormat                                        = DXGI_FORMAT_UNKNOWN;
	gfxPsod.BlendState.AlphaToCoverageEnable                 = FALSE;
	gfxPsod.BlendState.IndependentBlendEnable                = FALSE;
	gfxPsod.BlendState.RenderTarget[0].BlendEnable           = TRUE;
	gfxPsod.BlendState.RenderTarget[0].SrcBlend              = D3D12_BLEND_ONE;
	gfxPsod.BlendState.RenderTarget[0].DestBlend             = D3D12_BLEND_ZERO;
	gfxPsod.BlendState.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	gfxPsod.BlendState.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ZERO;
	gfxPsod.BlendState.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	gfxPsod.BlendState.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	gfxPsod.BlendState.RenderTarget[0].LogicOpEnable         = FALSE;
	gfxPsod.BlendState.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
	gfxPsod.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	gfxPsod.DepthStencilState.DepthEnable                    = FALSE;
	gfxPsod.DepthStencilState.DepthFunc                      = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxPsod.DepthStencilState.DepthWriteMask                 = D3D12_DEPTH_WRITE_MASK_ZERO;
	gfxPsod.DepthStencilState.StencilEnable                  = FALSE;
	gfxPsod.DepthStencilState.StencilReadMask                = 0;
	gfxPsod.DepthStencilState.StencilWriteMask               = 0;
	gfxPsod.DepthStencilState.FrontFace.StencilFunc          = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxPsod.DepthStencilState.FrontFace.StencilDepthFailOp   = D3D12_STENCIL_OP_KEEP;
	gfxPsod.DepthStencilState.FrontFace.StencilFailOp        = D3D12_STENCIL_OP_KEEP;
	gfxPsod.DepthStencilState.FrontFace.StencilPassOp        = D3D12_STENCIL_OP_KEEP;
	gfxPsod.DepthStencilState.BackFace.StencilFunc           = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxPsod.DepthStencilState.BackFace.StencilDepthFailOp    = D3D12_STENCIL_OP_KEEP;
	gfxPsod.DepthStencilState.BackFace.StencilFailOp         = D3D12_STENCIL_OP_KEEP;
	gfxPsod.DepthStencilState.BackFace.StencilPassOp         = D3D12_STENCIL_OP_KEEP;
	gfxPsod.SampleMask                                       = 0xFFFFFFFF;
	gfxPsod.SampleDesc.Count                                 = 1;
	gfxPsod.SampleDesc.Quality                               = 0;
	gfxPsod.NodeMask                                         = 0;
	gfxPsod.CachedPSO.CachedBlobSizeInBytes                  = 0;
	gfxPsod.CachedPSO.pCachedBlob                            = nullptr;
	gfxPsod.Flags                                            = D3D12_PIPELINE_STATE_FLAG_NONE;

	m_Device->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(result));

	LOG("Created pipeline state: " << result);
}

void DX12PipelineInjector::InjectShaders(ID3D12GraphicsCommandList *commandList, ID3D12Resource *backBufferResource,
                                         D3D12_CPU_DESCRIPTOR_HANDLE backBufferView,
                                         D3D12_RESOURCE_STATES backBufferState, std::string_view pixelShader,
                                         std::string_view vertexShader)
{
	if (!IsInit())
		return;

	const auto hash = GetHash(pixelShader, vertexShader);

	ComPointer<ID3D12PipelineState> pipelineState;

	if (!m_PSOMap.contains(hash))
	{
		CreatePostProcessPSO(pixelShader, vertexShader, &pipelineState);
		m_PSOMap[hash] = pipelineState;
	}

	pipelineState = m_PSOMap[hash];

	if (!pipelineState.Get())
		return;

	AddBarrier(commandList, m_SceneTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	           D3D12_RESOURCE_STATE_COPY_DEST);
	AddBarrier(commandList, backBufferResource, backBufferState, D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->CopyResource(m_SceneTexture.Get(), backBufferResource);
	AddBarrier(commandList, m_SceneTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
	           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	AddBarrier(commandList, backBufferResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->OMSetRenderTargets(1, &backBufferView, false, nullptr);
	float color[4] = { 0.0f, 0.3f, 0.7f, 1.0f };
	commandList->ClearRenderTargetView(backBufferView, color, 0, nullptr);

	commandList->SetPipelineState(pipelineState.Get());

	commandList->SetGraphicsRootSignature(m_PostProcessRootSignature.Get());

	ID3D12DescriptorHeap *heaps[] = { m_SRVHeap.Get() };
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(0, m_SRVHeap->GetGPUDescriptorHandleForHeapStart());
	// rootsigs only support 32 bit values
	ULONG ticks = GetTickCount();
	union FloatBits
	{
		float f;
		ULONG i;
	};

	FloatBits rand;
	rand.f = g_Random.GetRandomFloat(0.f, 1.f);

	ULONG constants[] = { ticks, rand.i };

	commandList->SetGraphicsRoot32BitConstants(1, 2, constants, 0);

	D3D12_VERTEX_BUFFER_VIEW vbView;
	vbView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes    = 6 * sizeof(Vertex);
	vbView.StrideInBytes  = sizeof(Vertex);
	commandList->IASetVertexBuffers(0, 1, &vbView);

	D3D12_VIEWPORT vp[1];
	vp[0].Width    = (float)m_Width;
	vp[0].Height   = (float)m_Height;
	vp[0].MinDepth = 0;
	vp[0].MaxDepth = 1;
	vp[0].TopLeftX = 0;
	vp[0].TopLeftY = 0;
	commandList->RSSetViewports(1, vp);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);

	AddBarrier(commandList, backBufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, backBufferState);
}