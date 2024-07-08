// AMD AMDUtils code
//
// Copyright(c) 2017 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <windows.h>
#include <d3dx12.h>

// Simulates DX11 style static buffers. For dynamic buffers please see 'DynamicBufferRingDX12.h'
//
// This class allows suballocating small chuncks of memory from a huge buffer that is allocated on creation
// This class is specialized in vertex buffers.
//
#include <mutex>

class StaticBufferPoolDX12 {
  public:
    void OnCreate(ID3D12Device* pDevice, DWORD totalMemSize, bool bUseVidMem, UINT node = 0, UINT nodemask = 0);
    void OnDestroy();

    bool AllocVertexBuffer(DWORD numbeOfVertices, UINT strideInBytes, void **pData, D3D12_VERTEX_BUFFER_VIEW *pView);
    bool AllocIndexBuffer(DWORD numbeOfIndices, UINT strideInBytes, void **pData, D3D12_INDEX_BUFFER_VIEW *pIndexView);

    void UploadData(ID3D12GraphicsCommandList *pCmdList);
    void FreeUploadHeap();

  private:
    std::mutex       m_mutex;

    ID3D12Device    *m_pDevice;
    ID3D12Resource  *m_pMemBuffer;
    ID3D12Resource  *m_pSysMemBuffer;
    ID3D12Resource  *m_pVidMemBuffer;

    char            *m_pData;
    DWORD            m_memOffset;
    DWORD            m_totalMemSize;

    bool             m_bUseVidMem;
};


