#include "D3DClass.h"
#include <minwinbase.h>

/*
	Initialize all the variables
*/
D3DClass::D3DClass()
{
	m_device = nullptr;
	m_commandQueue = nullptr;
	m_swapChain = nullptr;
	m_renderTargetViewHeap = nullptr;
	m_backBufferRenderTarget[0] = nullptr;
	m_backBufferRenderTarget[1] = nullptr;
	m_commandAllocator = nullptr;
	m_commandList = nullptr;
	m_pipelineState = nullptr;
	m_fence = nullptr;
	m_fenceEvent = nullptr;
	m_vSyncEnabled = false;
	m_bufferIndex = 0;
	m_fenceValue = 0;
	m_videoCardMemory = 0;
}

D3DClass::~D3DClass()
{
	
}

/*
	Initialize and setup DirectX 12
	Create the device which will help to manage rendering
	Create the commandqueue which will work trough all the rendering commands
	Create the factory to allow interaction with the graphics card
	Get the primary graphics card
	Get the refreshrate of the monitor
	Get the graphics card name and its memory
	Initialize the Swapchain which will handle the writing and clearing the two back buffers
	Setup the render target view so we can render to the screen
	Get the current buffer to draw to
	Create the commandallocator so we can allocate enough memory for the commands
	Create commandlist to send the commands to the commandqueue which is attached to the graphics card
	Create a fence and an event for GPU synchronization
*/
bool D3DClass::Initialize(int _screenHeight, int _screenWidth, HWND _windowHandle, bool _vSync, bool _fullscreen)
{
	m_vSyncEnabled = _vSync;
	HRESULT result = 0;

	if(!CreateDevice(result, _windowHandle))
	{
		return false;
	}

	if (!CreateCommandQueue(result))
	{
		return false;
	}

	//	Create DirectX graphics interface factory
	IDXGIFactory4* factory;
	result = CreateDXGIFactory1(_uuidof(IDXGIFactory4), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	//	Using the factory create an adapter for the primary graphics card
	IDXGIAdapter* adapter;
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	unsigned int denominator = 0;
	unsigned int numerator = 0;
	if (!GetRefreshRateOfMonitor(result, numerator, denominator, adapter, _screenHeight, _screenWidth))
	{
		return false;
	}

	if (!GetNameAndVideoCardMemory(result, adapter))
	{
		return false;
	}

	if (!InitializeSwapChain(result, numerator, denominator, factory, _windowHandle, _screenHeight, _screenWidth, _fullscreen))
	{
		return false;
	}

	if (!SetupRenderTargetView(result))
	{
		return false;
	}

	//	Get the current buffer to be drawing to
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//	Create the commandallocator, allocating memory for the lsit of commands that we send to the GPU each frame
	result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, _uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocator);
	if (FAILED(result))
	{
		return false;
	}

	//	Create the commandlist which sends the commands to the commandqueue to be rendered by the GPU
	//	This belongs somewhere else later one, we will be using multiple commandlists which are working parallel and multi threaded
	result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, _uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList);
	if (FAILED(result))
	{
		return false;
	}

	//	Close the commandlist because it is created in a recording state
	result = m_commandList->Close();
	if (FAILED(result))
	{
		return false;
	}

	//	Creating a fence, which will notify us when the GPU is ready with the rendering of the commandlist
	//	We are able to synchronize GPU and CPU with the fence
	//	Create a fence for GPU synchronization
	result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, _uuidof(ID3D12Fence), (void**)&m_fence);
	if (FAILED(result))
	{
		return false;
	}

	//	Create an event object for the fence which will give us feedback on finished rendering
	m_fenceEvent = CreateEventEx(nullptr, nullptr, FALSE, EVENT_ALL_ACCESS);
	if (m_fenceEvent == nullptr)
	{
		return false;
	}

	//	Start the fence at position 1
	m_fenceValue = 1;

	return true;
}

bool D3DClass::Render()
{
	HRESULT result = m_commandAllocator->Reset();
	if (FAILED(result))
	{
		return false;
	}

	result = m_commandList->Reset(m_commandAllocator, m_pipelineState);
	if (FAILED(result))
	{
		return false;
	}

	D3D12_RESOURCE_BARRIER barrier;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_backBufferRenderTarget[m_bufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	m_commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (m_bufferIndex == 1)
	{
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	}

	m_commandList->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, nullptr);

	float color[4];
	color[0] = 0.5;
	color[1] = 0.5;
	color[2] = 0.5;
	color[3] = 1.0;

	m_commandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, nullptr);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_commandList->ResourceBarrier(1, &barrier);

	result = m_commandList->Close();
	if (FAILED(result))
	{
		return false;
	}

	ID3D12CommandList* pCommandLists[1];
	pCommandLists[0] = m_commandList;
	
	m_commandQueue->ExecuteCommandLists(1, pCommandLists);

	if (m_vSyncEnabled)
	{
		result = m_swapChain->Present(1, 0);
		if (FAILED(result))
		{
			return false;
		}
	}
	else
	{
		result = m_swapChain->Present(0, 0);
		if (FAILED(result))
		{
			return false;
		}
	}

	unsigned long long fenceToWaitFor = m_fenceValue;

	result = m_commandQueue->Signal(m_fence, fenceToWaitFor);
	if (FAILED(result))
	{
		return false;
	}

	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fenceToWaitFor)
	{
		result = m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
		if (FAILED(result))
		{
			return false;
		}

		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_bufferIndex == 0 ? m_bufferIndex = 1 : m_bufferIndex = 0;

	return true;
}

/*
	Release all the memory and clean up the pointer from the private member variables
	Force the swapchain to change to windowed mode, else there will be thrown multiple exceptions
*/
void D3DClass::Shutdown()
{
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, nullptr);
	}
	int error = CloseHandle(m_fenceEvent);
	if (error == 0)
	{

	}
	if (m_fence)
	{
		m_fence->Release();
		m_fence = nullptr;
	}
	if (m_pipelineState)
	{
		m_pipelineState->Release();
		m_pipelineState = nullptr;
	}
	if (m_commandList)
	{
		m_commandList->Release();
		m_commandList = nullptr;
	}
	if (m_commandAllocator)
	{
		m_commandAllocator->Release();
		m_commandAllocator = nullptr;
	}
	if (m_backBufferRenderTarget[0])
	{
		m_backBufferRenderTarget[0]->Release();
		m_backBufferRenderTarget[0] = nullptr;
	}
	if (m_backBufferRenderTarget[1])
	{
		m_backBufferRenderTarget[1]->Release();
		m_backBufferRenderTarget[1] = nullptr;
	}
	if (m_renderTargetViewHeap)
	{
		m_renderTargetViewHeap->Release();
		m_renderTargetViewHeap = nullptr;
	}
	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}
	if (m_commandQueue)
	{
		m_commandQueue->Release();
		m_commandQueue = nullptr;
	}
	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
}


/*
We want to create the device
First we want to set the DirectX12 version which we want to use (backwards compatible)
We are able to use DirectX12 but without the features our graphics card can't handle

With the device we can perform further initializiation we are going to need later on
*/
bool D3DClass::CreateDevice(HRESULT _result, HWND _windowHandle)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;

	_result = D3D12CreateDevice(nullptr, featureLevel, _uuidof(ID3D12Device), (void**)&m_device);
	if (FAILED(_result))
	{
		MessageBox(_windowHandle, L"Could not create a DirectX 12.1 device. Video card does not support DirectX 12.1.", L"DirectX Device Failure", MB_OK);
		return false;
	}

	return true;
}

/*
We are going to create the commandqueue which will be executing our commandlist
Each frame the rendering will be put into the commandlist which will be passed to the commandqueue and finally executed on the GPU
Generally we have one commandqueue per GPU
NodeMask = 0 specifies using a single GPU
*/
bool D3DClass::CreateCommandQueue(HRESULT _result)
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));

	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	_result = m_device->CreateCommandQueue(&commandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_commandQueue);
	if (FAILED(_result))
	{
		return false;
	}

	return true;
}

/*
	Before initializing the swap chain we have to get the refresh rate of the graphics card/monitor
	Query for enumerator and denominator and pass them to DirectX during the setup, so they will be calculated properly
	If we don't do this and just set the values, they might not exist on the computer, DirectX will throw errors and perform a buffer copy instead of a flip
	This will decrease our performance
*/
bool D3DClass::GetRefreshRateOfMonitor(HRESULT _result, unsigned int& _numerator, unsigned int& _denominator, IDXGIAdapter* _adapter, int _screenHeight, int _screenWidth)
{
	//	get the primary adapter output(monitor)
	IDXGIOutput* adapterOutput;
	_result = _adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(_result))
	{
		return false;
	}

	//	get the number of modes fitting the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
	unsigned int numModes;
	_result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (FAILED(_result))
	{
		return false;
	}

	//	create a list to hold all possible display modes for this monitor/video card combination
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	//	fill the displaymodelist
	_result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(_result))
	{
		return false;
	}

	//	go trough all display modes and find the one that matches the screen height and width
	//	when we've found a match store the numerator and denominator
	for (unsigned int i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Height == static_cast<unsigned int>(_screenHeight))
		{
			if (displayModeList[i].Width == static_cast<unsigned int>(_screenWidth))
			{
				_numerator = displayModeList[i].RefreshRate.Numerator;
				_denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	//	release displayModeList and adapterOutput 
	delete[] displayModeList;
	displayModeList = nullptr;

	adapterOutput->Release();
	adapterOutput = nullptr;

	return true;
}

/*
	First get the description of the graphics card
	Store the dedicated graphics card memory in megabytes
	Convert the name of the video card to a character array and store it in m_videoCardDescription
	Finally release the adapter
*/
bool D3DClass::GetNameAndVideoCardMemory(HRESULT _result, IDXGIAdapter* _adapter)
{
	DXGI_ADAPTER_DESC adapterDesc;
	_result = _adapter->GetDesc(&adapterDesc);
	if (FAILED(_result))
	{
		return false;
	}

	m_videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	size_t stringLength;
	int error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	_adapter->Release();
	_adapter = nullptr;

	return true;
}

/*
Initialize SwapChain = Swapchain means creating two buffers,
One is beeing written in the background and one is showing the written data to the monitor,
After finished writing swap them and do the same

Create a new swapchain description, empty its memory and fill it out
Then create a new swapchain with this description, the factory and the commandqueue which is paired with the graphics card
Get the version 3 interface of the swapchain so we can access newer methods
Reference it to our member variable swapchain
Free the factory and the local swapchain because we do not need them anymore
*/
bool D3DClass::InitializeSwapChain(HRESULT _result, unsigned int _numerator, unsigned int _denominator, IDXGIFactory4* _factory, HWND _windowHandle, int _screenHeight, int _screenWidth, bool _fullscreen)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//	Use double buffering
	swapChainDesc.BufferCount = 2;

	//	Set the height and width of the back buffer in the SwapChain
	swapChainDesc.BufferDesc.Height = _screenHeight;
	swapChainDesc.BufferDesc.Width = _screenWidth;

	//	Set a 32-bit surface for the back buffers
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//	Set usage of the back buffers to be render target outputs
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//	Set the swap effect to discard the previous buffer content after swapping
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//	Set the handle for the window to render to
	swapChainDesc.OutputWindow = _windowHandle;

	//	Set to fullscreen or windowed mode
	if (_fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	//	Set the refresh rate
	//	How often the back buffer draws to the screen
	//	If vsync is enabled lock it to the systemsettings (default: 60hz)
	//	This will render 60 times a second
	//	If vsync is not enabled do not lock it to the systemsettings and draw as much as we can (can cause visual effects)
	if (m_vSyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = _numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = _denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	//	Turn multisampling off
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	//	Set the scan line ordering and scaling to unspecified
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//	We do not want to set the advanced flags
	swapChainDesc.Flags = 0;

	IDXGISwapChain* swapChain;
	_result = _factory->CreateSwapChain(m_commandQueue, &swapChainDesc, &swapChain);
	if (FAILED(_result))
	{
		return false;
	}

	_result = swapChain->QueryInterface(_uuidof(IDXGISwapChain3), (void**)&m_swapChain);
	if (FAILED(_result))
	{
		return false;
	}

	swapChain = nullptr;

	_factory->Release();
	_factory = nullptr;

	return true;
}

/*
RenderTargetViews allow GPU to use the backbuffers as resources to render to
First create a descriptor heap description which holds the two backbuffers in memory
Get a handle to the descriptor heap memory and create a view by using the pointer to that memory

Common theme for all resource binding in DirectX12

Create the heap description, clear its memory and fill it out
Create the render target view heap for the back buffers
Get a handle to the starting memory location in the render target view heap to identify where the render target views will be located
Get the size of the memory location for the render target view descriptors
Get a pointer to the first back buffer from the swap chain
Create a render target view for the first back buffer
Increment the view handle to the next descriptor location in the heap
Get a pointer to the second back buffer from the swap chain
Create a render target view fot he second back buffer
*/
bool D3DClass::SetupRenderTargetView(HRESULT _result)
{
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
	ZeroMemory(&renderTargetViewHeapDesc, sizeof(renderTargetViewHeapDesc));

	//	Set the number of descriptors to two for our two back buffers
	renderTargetViewHeapDesc.NumDescriptors = 2;

	//	Set the type to render target view
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	_result = m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, _uuidof(ID3D12DescriptorHeap), (void**)&m_renderTargetViewHeap);
	if (FAILED(_result))
	{
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	_result = m_swapChain->GetBuffer(0, _uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[0]);
	if (FAILED(_result))
	{
		return false;
	}

	m_device->CreateRenderTargetView(m_backBufferRenderTarget[0], nullptr, renderTargetViewHandle);

	renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

	_result = m_swapChain->GetBuffer(1, _uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[1]);
	if (FAILED(_result))
	{
		return false;
	}

	m_device->CreateRenderTargetView(m_backBufferRenderTarget[1], nullptr, renderTargetViewHandle);

	return true;
}