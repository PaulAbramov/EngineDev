#pragma once

#pragma region Direct X 12 linking
#pragma comment(lib, "d3d12.lib")			// contains all Direct3D functions for setting up and drawing in Direct X 12
#pragma comment(lib, "dxgi.lib")			// contains tools to interface with hardware
#pragma comment(lib, "d3dcompiler.lib")		// contains functions to compile shaders
#pragma endregion

#pragma region includes
#include <d3d12.h>
#include <dxgi1_4.h>
#pragma endregion

class D3DClass
{
public:
	D3DClass();
	~D3DClass();

	bool Initialize(int _screenHeight, int _screenWidth, HWND _windowHandle, bool _vSync, bool _fullscreen);
	void Shutdown();

	bool Render();

private:
	bool m_vSyncEnabled;
	char m_videoCardDescription[128];
	unsigned int m_bufferIndex;
	unsigned long long m_fenceValue;
	unsigned int m_videoCardMemory;

	HANDLE m_fenceEvent;

	ID3D12Device* m_device;
	ID3D12CommandQueue* m_commandQueue;
	ID3D12DescriptorHeap* m_renderTargetViewHeap;
	ID3D12Resource* m_backBufferRenderTarget[2];
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12PipelineState* m_pipelineState;
	ID3D12Fence* m_fence;

	IDXGISwapChain3* m_swapChain;

	bool CreateDevice(HRESULT _result, HWND _windowHandle);
	bool CreateCommandQueue(HRESULT _result);
	static bool GetRefreshRateOfMonitor(HRESULT _result, unsigned int& _numerator, unsigned int& _denominator, IDXGIAdapter* _adapter, int _screenHeight, int _screenWidth);
	bool GetNameAndVideoCardMemory(HRESULT _result, IDXGIAdapter* _adapter);
	bool InitializeSwapChain(HRESULT _result, unsigned int _numerator, unsigned int _denominator, IDXGIFactory4* _factory, HWND _windowHandle, int _screenHeight, int _screenWidth, bool _fullscreen);
	bool SetupRenderTargetView(HRESULT _result);
};