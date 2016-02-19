#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFont.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Font\SignedDistanceFontRenderer.h"
#include "ToyGE\Kernel\Asset.h"

namespace ToyGE
{
	Ptr<FontRenderer> SignedDistanceFieldFont::CreateRenderer()
	{
		return std::make_shared<SignedDistanceFontRenderer>(shared_from_this()->Cast<SignedDistanceFieldFont>());
	}

	inline bool Remove(int2 u, int2 v, int2 w, int32_t r)
	{
		auto u2 = u * u;
		auto v2 = v * v;
		auto w2 = w * w;
		return (w.x() - v[0]) * (v2.x() - u2.x() - 2 * r * (v.y() - u.y()) + v2.y() - u2.y())
			>= (v.x() - u.x()) * (w2.x() - v2.x() - 2 * r * (w.y() - v.y()) + w2.y() - v2.y());
	}

	inline float GetIntersectionX(int2 u, int2 v, int32_t r)
	{
		auto u2 = u * u;
		auto v2 = v * v;
		return (float)(v2.x() - u2.x() - 2 * r * (v.y() - u.y()) + v2.y() - u2.y())
			/ (float)(2 * (v.x() - u.x()));
	}

	inline float Dist(int2 u, int2 v)
	{
		auto disti2 = u - v;
		return std::sqrt((float)dot(disti2 * disti2, int2(1)));
	}

	void SignedDistanceFieldFont::InitDistanceData(int2 glyphSize, int3 glyphMapDims, std::shared_ptr<uint8_t> & data, std::vector<RenderDataDesc> & dataDescs)
	{
		// Compute nearest distance	
		int32_t glyphWidth = glyphSize.x();
		int32_t glyphHeight = glyphSize.y();
		int32_t glyphMapDataPitch = glyphMapDims.y() * glyphWidth;
		int32_t glyphMapDataSlice = glyphMapDims.x() * glyphHeight * glyphMapDataPitch;

		auto outData = MakeBufferedDataShared(glyphMapDataSlice * glyphMapDims.z() * 2);

		auto numGlyphs = GetAsset()->GetNumGlyphs();
		for (int32_t glyphIndex = 0; glyphIndex < numGlyphs; ++glyphIndex)
		{
			int32_t glyphPosX = glyphIndex % _glyphMapCols;
			int32_t glyphPosY = glyphIndex / _glyphMapCols;

			int32_t glyphDataStart = glyphPosY * glyphHeight * glyphMapDataPitch + glyphPosX * glyphWidth;
			uint8_t * pGlyphDataStart = data.get() + glyphDataStart;
			uint8_t * pDistanceDataStart = outData.get() + glyphDataStart * 2;

			// Edge
			std::vector<std::vector<bool>> edge(glyphHeight);
			std::vector<std::vector<int32_t>> edgePos(glyphHeight);
			for (int32_t row = 0; row < glyphHeight; ++row)
			{
				edge[row].resize(glyphWidth);

				for (int32_t col = 0; col < glyphWidth; ++col)
				{
					if (*(pGlyphDataStart + row * glyphMapDataPitch + col) > 0)
					{
						// 8 neighbors
						bool bEdge = false;
						for (int32_t i = -1; i <= 1 && !bEdge; ++i)
						{
							for (int32_t j = -1; j <= 1 && !bEdge; ++j)
							{
								if (i == 0 && j == 0)
									continue;

								int32_t x = col + j;
								int32_t y = row + i;
								if (x >= 0 && x < glyphWidth &&
									y >= 0 && y < glyphHeight)
								{
									if (*(pGlyphDataStart + y * glyphMapDataPitch + x) == 0)
										bEdge = true;
								}
								else
									bEdge = true;
							}
						}
						edge[row][col] = bEdge;
					}
					else
						edge[row][col] = false;

					if (edge[row][col])
						edgePos[row].push_back(col);


				}
			}

			// Up to Bottom
			std::vector<std::vector<int2>> l0(glyphHeight);
			{
				std::vector<int2> candidates;
				for (int32_t x = 0; x < glyphWidth; ++x)
				{
					if (edge[0][x])
						candidates.push_back(int2(x, 0));
				}
				l0[0] = candidates;

				for (int32_t row = 1; row < glyphHeight; ++row)
				{
					auto & l_r = l0[row];
					if (candidates.size() > 2)
					{
						l_r.push_back(candidates[0]);
						l_r.push_back(candidates[1]);
						int32_t c = 2;
						while (c < (int32_t)candidates.size())
						{
							while (l_r.size() >= 2 && Remove(l_r[l_r.size() - 2], l_r.back(), candidates[c], row))
								l_r.pop_back();
							l_r.push_back(candidates[c]);
							++c;
						}
					}
					else
						l_r = candidates;

					// update candidates
					int32_t c = 0;
					std::vector<int2> newCandidates;
					for (int32_t x = 0; x < glyphWidth; ++x)
					{
						if (c < (int32_t)candidates.size() && candidates[c].x() == x)
						{
							if (edge[row][x])
								newCandidates.push_back(int2(x, row));
							else
								newCandidates.push_back(candidates[c]);
							++c;
						}
						else
						{
							if (edge[row][x])
								newCandidates.push_back(int2(x, row));
						}
					}
					candidates = newCandidates;
				}
			}

			// Bottom to up
			std::vector<std::vector<int2>> l1(glyphHeight);
			{
				std::vector<int2> candidates;
				for (int32_t x = 0; x < glyphWidth; ++x)
				{
					if (edge[glyphHeight - 1][x])
						candidates.push_back(int2(x, glyphHeight - 1));
				}
				l1[0] = candidates;

				for (int32_t row = glyphHeight - 2; row >= 0; --row)
				{
					auto & l_r = l1[glyphHeight - row - 1];
					if (candidates.size() > 2)
					{
						l_r.push_back(candidates[0]);
						l_r.push_back(candidates[1]);
						int32_t c = 2;
						while (c < (int32_t)candidates.size())
						{
							while (l_r.size() >= 2 && Remove(l_r[l_r.size() - 2], l_r.back(), candidates[c], row))
								l_r.pop_back();
							l_r.push_back(candidates[c]);
							++c;
						}
					}
					else
						l_r = candidates;

					// update candidates
					int32_t c = 0;
					std::vector<int2> newCandidates;
					for (int32_t x = 0; x < glyphWidth; ++x)
					{
						if (c < (int32_t)candidates.size() && candidates[c].x() == x)
						{
							if (edge[row][x])
								newCandidates.push_back(int2(x, row));
							else
								newCandidates.push_back(candidates[c]);
							++c;
						}
						else
						{
							if (edge[row][x])
								newCandidates.push_back(int2(x, row));
						}
					}
					candidates = newCandidates;
				}
			}

			// Find nearest
			float maxDist = Dist(int2(0, 0), int2(glyphWidth - 1, glyphHeight - 1));
			for (int32_t row = 0; row < glyphHeight; ++row)
			{
				std::vector<float> dists(glyphWidth, FLT_MAX);
				std::vector<int2> nearestEdgePos(glyphWidth, 255);

				if (row != 0)
				{
					// search in l0
					auto & l0_r = l0[row];
					if (l0_r.size() > 0)
					{
						int32_t l0Index = 0;
						int32_t end = -1;
						if (l0_r.size() > 1)
						{
							auto x = std::ceil(GetIntersectionX(l0_r[0], l0_r[1], row));
							if (x <= 0.0f)
							{
								l0Index = 1;
								if (l0_r.size() > 2)
									end = (int32_t)std::ceil(GetIntersectionX(l0_r[1], l0_r[2], row));
							}
							else
								end = (int32_t)x;
						}

						for (int32_t x = 0; x < glyphWidth; ++x)
						{
							if (x < end)
							{
								//auto & nearest = l1_r[l1Index];
								/*auto disti2 = nearest - int2(x, 0);
								dists[x] = std::min<float>( dists[x], std::sqrt((float)dot(disti2 * disti2, int2(1))) );*/
								//dists[x] = std::min<float>(dists[x], Dist(l0_r[l0Index], int2(x, row)));
								float d = Dist(l0_r[l0Index], int2(x, row));
								if (d < dists[x])
								{
									dists[x] = d;
									nearestEdgePos[x] = l0_r[l0Index];
								}
							}
							else
							{
								if (l0Index < (int32_t)l0_r.size() - 1)
								{
									end = (int32_t)std::ceil(GetIntersectionX(l0_r[l0Index], l0_r[l0Index + 1], row));
									++l0Index;
								}
								else
									end = glyphWidth;

								//dists[x] = std::min<float>(dists[x], Dist(l0_r[l0Index], int2(x, row)));
								float d = Dist(l0_r[l0Index], int2(x, row));
								if (d < dists[x])
								{
									dists[x] = d;
									nearestEdgePos[x] = l0_r[l0Index];
								}
							}
						}
					}
				}

				// search edges in row
				if (edgePos[row].size() > 0)
				{
					int32_t nearestIndex = 0;
					float bound = (float)edgePos[row][nearestIndex];
					for (int32_t x = 0; x < glyphWidth; ++x)
					{
						if ((float)x <= bound)
						{
							//dists[x] = (float)std::abs(edgePos[row][nearestIndex] - x);
							float d = (float)std::abs(edgePos[row][nearestIndex] - x);
							if (d < dists[x])
							{
								dists[x] = d;
								nearestEdgePos[x] = int2(edgePos[row][nearestIndex], row);
							}
						}
						else
						{
							if (bound == (float)edgePos[row][nearestIndex])
							{
								if (nearestIndex < (int32_t)edgePos[row].size() - 1)
								{
									if (edgePos[row][nearestIndex + 1] - edgePos[row][nearestIndex] > 1)
										bound = (float)(edgePos[row][nearestIndex] + edgePos[row][nearestIndex + 1]) * 0.5f;
									else
									{
										bound = (float)edgePos[row][nearestIndex + 1];
										++nearestIndex;
									}
								}
								else
									bound = (float)glyphWidth;
							}
							else
							{
								++nearestIndex;
								bound = (float)edgePos[row][nearestIndex];
							}

							//dists[x] = (float)std::abs(edgePos[row][nearestIndex] - x);
							float d = (float)std::abs(edgePos[row][nearestIndex] - x);
							if (d < dists[x])
							{
								dists[x] = d;
								nearestEdgePos[x] = int2(edgePos[row][nearestIndex], row);
							}
						}
					}
				}

				// search in l1
				if (row != glyphHeight - 1)
				{
					auto & l1_r = l1[glyphHeight - 1 - row];
					if (l1_r.size() > 0)
					{
						int32_t l1Index = 0;
						int32_t end = -1;
						if (l1_r.size() > 1)
						{
							auto x = std::ceil(GetIntersectionX(l1_r[0], l1_r[1], row));
							if (x <= 0.0f)
							{
								l1Index = 1;
								if (l1_r.size() > 2)
									end = (int32_t)std::ceil(GetIntersectionX(l1_r[1], l1_r[2], row));
							}
							else
								end = (int32_t)x;
						}

						for (int32_t x = 0; x < glyphWidth; ++x)
						{
							if (x < end)
							{
								//auto & nearest = l1_r[l1Index];
								/*auto disti2 = nearest - int2(x, 0);
								dists[x] = std::min<float>( dists[x], std::sqrt((float)dot(disti2 * disti2, int2(1))) );*/
								//dists[x] = std::min<float>(dists[x], Dist(l1_r[l1Index], int2(x, row)));
								float d = Dist(l1_r[l1Index], int2(x, row));
								if (d < dists[x])
								{
									dists[x] = d;
									nearestEdgePos[x] = l1_r[l1Index];
								}
							}
							else
							{
								if (l1Index < (int32_t)l1_r.size() - 1)
								{
									end = (int32_t)std::ceil(GetIntersectionX(l1_r[l1Index], l1_r[l1Index + 1], row));
									++l1Index;
								}
								else
									end = glyphWidth;

								//dists[x] = std::min<float>(dists[x], Dist(l1_r[l1Index], int2(x, row)));
								float d = Dist(l1_r[l1Index], int2(x, row));
								if (d < dists[x])
								{
									dists[x] = d;
									nearestEdgePos[x] = l1_r[l1Index];
								}
							}
						}
					}
				}


				for (int32_t x = 0; x < glyphWidth; ++x)
				{
					/*uint8_t d = 255;
					if (dists[x] != FLT_MAX)
						d = (uint8_t)std::round( dists[x] / maxDist * 255.0f );
					*(pGlyphDataStart + row * glyphMapDataPitch + x) = d;*/

					*(pDistanceDataStart + row * glyphMapDataPitch * 2 + x * 2) = (uint8_t)nearestEdgePos[x].x();
					*(pDistanceDataStart + row * glyphMapDataPitch * 2 + x * 2 + 1) = (uint8_t)nearestEdgePos[x].y();
				}
			}

		}

		uint8_t * pData = outData.get();
		for (auto & desc : dataDescs)
		{
			desc.pData = pData;
			desc.rowPitch = glyphMapDataPitch * 2;
			desc.slicePitch = glyphMapDataSlice * 2;

			pData += desc.slicePitch;
		}

		data = outData;
	}

	void SignedDistanceFieldFont::Init()
	{
		if (!GetAsset())
			return;

		if (LoadCache())
			return;

		auto & fontFace = GetAsset()->GetFontFace();
		if (!fontFace)
			return;

		_glyphSize *= 4;
		borderSpace = 4;
		BitmapFont::InitGlyphMapDataAndRenderInfo(true);
		BitmapFont::InitGlyphMapTex(false);
		auto coverageTex = _glyphRenderMapTex;
		auto coverageNumGlyphRows = _glyphMapRows;
		auto coverageNumGlyphCols = _glyphMapCols;
		auto coverageNumGlyphSlices = _glyphMapSlices;
		_glyphMapData.reset();

		auto glyphWidth = _glyphSize.x();
		auto glyphHeight = _glyphSize.y();
		
		float2 glyphSize = float2((float)glyphWidth, (float)glyphHeight);
		float4 coverageTexSize = coverageTex->GetTexSize();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetBlendState(BlendStateTemplate<>::Get());

		// Compute gradient
		auto gradientTexDesc = coverageTex->GetDesc();
		gradientTexDesc.format = RENDER_FORMAT_R16G16_FLOAT;
		auto gradientTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		gradientTex->SetDesc(gradientTexDesc);
		gradientTex->Init({});

		auto computeGradientPS = Shader::FindOrCreate<SignedDistanceFieldFontComputeGradientPS>();
		for (int32_t sliceIndex = 0; sliceIndex < gradientTexDesc.arraySize; ++sliceIndex)
		{
			computeGradientPS->SetScalar("coverageTexSize", coverageTexSize);
			computeGradientPS->SetScalar("glyphSize", glyphSize);
			computeGradientPS->SetSRV("coverageTex", coverageTex->GetShaderResourceView(0, 1, sliceIndex, 1));
			computeGradientPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			computeGradientPS->Flush();

			DrawQuad({ gradientTex->GetRenderTargetView(0, sliceIndex, 1) });
		}

		// Compute nearest edge
		std::shared_ptr<uint8_t> distanceData = MakeBufferedDataShared(coverageTex->GetDataSize());
		std::vector<RenderDataDesc> distanceDataDataDesc;
		coverageTex->Dump(distanceData.get(), distanceDataDataDesc);
		InitDistanceData(_glyphSize, int3(coverageNumGlyphRows, coverageNumGlyphCols, coverageNumGlyphSlices), distanceData, distanceDataDataDesc);

		auto distanceTexDesc = coverageTex->GetDesc();
		distanceTexDesc.format = RENDER_FORMAT_R8G8_UINT;
		auto distanceTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		distanceTex->SetDesc(distanceTexDesc);
		distanceTex->Init(distanceDataDataDesc);

		distanceData.reset();
		distanceDataDataDesc.clear();
		
		// Compute offsets
		auto distanceOffsetTexDesc = coverageTex->GetDesc();
		distanceOffsetTexDesc.format = RENDER_FORMAT_R16_FLOAT;
		auto distanceOffsetTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		distanceOffsetTex->SetDesc(distanceOffsetTexDesc);
		distanceOffsetTex->Init();

		auto computeDistanceOffsetPS = Shader::FindOrCreate<SignedDistanceFieldFontComputeSignedDistanceOffsetPS>();
		for (int32_t sliceIndex = 0; sliceIndex < distanceOffsetTexDesc.arraySize; ++sliceIndex)
		{
			computeDistanceOffsetPS->SetSRV("coverageTex", coverageTex->GetShaderResourceView(0, 1, sliceIndex, 1));
			computeDistanceOffsetPS->SetSRV("gradientTex", gradientTex->GetShaderResourceView(0, 1, sliceIndex, 1));
			computeDistanceOffsetPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			computeDistanceOffsetPS->Flush();

			DrawQuad({ distanceOffsetTex->GetRenderTargetView(0, sliceIndex, 1) });
		}

		// Signed distance field
		auto signedDistanceTexDesc = coverageTex->GetDesc();
		signedDistanceTexDesc.format = RENDER_FORMAT_R8_UNORM;
		auto signedDistanceTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		signedDistanceTex->SetDesc(signedDistanceTexDesc);
		signedDistanceTex->Init({});

		auto computeSignedDistanceFieldPS = Shader::FindOrCreate<SignedDistanceFieldFontComputeSignedDistancePS>();
		for (int32_t sliceIndex = 0; sliceIndex < signedDistanceTexDesc.arraySize; ++sliceIndex)
		{
			computeSignedDistanceFieldPS->SetScalar("coverageTexSize", coverageTexSize);
			computeSignedDistanceFieldPS->SetScalar("glyphSize", glyphSize);
			computeSignedDistanceFieldPS->SetSRV("coverageTex", coverageTex->GetShaderResourceView(0, 1, sliceIndex, 1));
			computeSignedDistanceFieldPS->SetSRV("distanceTex", distanceTex->GetShaderResourceView(0, 1, sliceIndex, 1));
			computeSignedDistanceFieldPS->SetSRV("distanceOffsetTex", distanceOffsetTex->GetShaderResourceView(0, 1, sliceIndex, 1));
			computeSignedDistanceFieldPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			computeSignedDistanceFieldPS->Flush();

			DrawQuad({ signedDistanceTex->GetRenderTargetView(0, sliceIndex, 1) });
		}

		coverageTex.reset();
		gradientTex.reset();
		distanceTex.reset();
		distanceOffsetTex.reset();

		// Down sample
		auto signedDistanceTexDown2x2Desc = signedDistanceTexDesc;
		signedDistanceTexDown2x2Desc.width /= 2;
		signedDistanceTexDown2x2Desc.height /= 2;
		auto signedDistanceTexDown2x2 = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		signedDistanceTexDown2x2->SetDesc(signedDistanceTexDown2x2Desc);
		signedDistanceTexDown2x2->Init({});

		for (int32_t sliceIndex = 0; sliceIndex < signedDistanceTexDown2x2Desc.arraySize; ++sliceIndex)
		{
			Transform(signedDistanceTex->GetShaderResourceView(0, 1, sliceIndex, 1), 
				signedDistanceTexDown2x2->GetRenderTargetView(0, sliceIndex, 1));
		}

		auto signedDistanceTexDown4x4Desc = signedDistanceTexDown2x2Desc;
		signedDistanceTexDown4x4Desc.width /= 2;
		signedDistanceTexDown4x4Desc.height /= 2;
		signedDistanceTexDown4x4Desc.mipLevels = 0;
		auto signedDistanceTexDown4x4 = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		signedDistanceTexDown4x4->SetDesc(signedDistanceTexDown4x4Desc);
		signedDistanceTexDown4x4->Init({});

		for (int32_t sliceIndex = 0; sliceIndex < signedDistanceTexDown4x4Desc.arraySize; ++sliceIndex)
		{
			Transform(signedDistanceTexDown2x2->GetShaderResourceView(0, 1, sliceIndex, 1),
				signedDistanceTexDown4x4->GetRenderTargetView(0, sliceIndex, 1));
		}
		
		_glyphRenderMapTex = signedDistanceTexDown4x4;
		_glyphRenderMapTex->GenerateMips();

		_glyphSize /= 4;

		_bNeedSaveCache = true;
		GetAsset()->SetDirty(true);
	}

	bool SignedDistanceFieldFont::LoadCache()
	{
		String cacheFilePath = GetAsset()->GetLoadPath().substr(0, GetAsset()->GetLoadPath().rfind('.')) + GetCacheExtension();
		if (!Global::GetPlatform()->FileExists(cacheFilePath))
			return false;

		auto cacheFile = Global::GetPlatform()->CreatePlatformFile(cacheFilePath, FILE_OPEN_READ);
		if (!cacheFile->IsValid())
			return false;

		auto reader = std::make_shared<FileReader>(cacheFile);

		_glyphRenderInfoTable.resize(GetAsset()->GetNumGlyphs());
		reader->ReadBytes(&_glyphRenderInfoTable[0], sizeof(_glyphRenderInfoTable[0]) * _glyphRenderInfoTable.size());

		TextureDesc texDesc;
		texDesc.width = reader->Read<int32_t>();
		texDesc.height = reader->Read<int32_t>();
		texDesc.arraySize = reader->Read<int32_t>();
		texDesc.depth = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		texDesc.mipLevels = reader->Read<int32_t>();
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		std::vector<std::shared_ptr<uint8_t>> data;
		std::vector<RenderDataDesc> dataDesc;
		for (int32_t arrayIndex = 0; arrayIndex < texDesc.arraySize; ++arrayIndex)
		{
			int32_t w = texDesc.width;
			for (int32_t mipLevel = 0; mipLevel < texDesc.mipLevels; ++mipLevel)
			{
				auto dataSize = reader->Read<size_t>();

				data.push_back(MakeBufferedDataShared(dataSize));

				reader->ReadBytes(data.back().get(), dataSize);

				RenderDataDesc desc;
				desc.pData = data.back().get();
				desc.rowPitch = w;
				dataDesc.push_back(desc);

				w = std::max<int32_t>(1, w / 2);
			}
		}

		_glyphRenderMapTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_glyphRenderMapTex->SetDesc(texDesc);
		_glyphRenderMapTex->Init(dataDesc);

		return true;
	}

	void SignedDistanceFieldFont::SaveCache()
	{
		String cacheFilePath = GetAsset()->GetLoadPath().substr(0, GetAsset()->GetLoadPath().rfind('.')) + GetCacheExtension();

		Global::GetPlatform()->MakeDirectoryRecursively(ParentPath(cacheFilePath));
		auto cacheFile = Global::GetPlatform()->CreatePlatformFile(cacheFilePath, FILE_OPEN_WRITE);
		if (!cacheFile->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot save signed distance field font cache! %s", cacheFilePath.c_str());
			return;
		}

		auto writer = std::make_shared<FileWriter>(cacheFile);

		writer->WriteBytes(&_glyphRenderInfoTable[0], sizeof(_glyphRenderInfoTable[0]) * _glyphRenderInfoTable.size());

		auto dumpTexDesc = _glyphRenderMapTex->GetDesc();
		dumpTexDesc.bindFlag = 0;
		dumpTexDesc.cpuAccess = CPU_ACCESS_READ;
		auto dumpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(_glyphRenderMapTex->GetType());
		dumpTex->SetDesc(dumpTexDesc);
		dumpTex->Init({});

		writer->Write<int32_t>(dumpTexDesc.width);
		writer->Write<int32_t>(dumpTexDesc.height);
		writer->Write<int32_t>(dumpTexDesc.arraySize);
		writer->Write<int32_t>(dumpTexDesc.mipLevels);

		for (int32_t arrayIndex = 0; arrayIndex < dumpTexDesc.arraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				_glyphRenderMapTex->CopyTo(dumpTex, mipLevel, arrayIndex, 0, 0, 0, mipLevel, arrayIndex);
			}
		}

		for (int32_t arrayIndex = 0; arrayIndex < dumpTexDesc.arraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				auto dataDesc = dumpTex->Map(MAP_READ, mipLevel, arrayIndex);

				auto mipSize = dumpTex->GetMipSize(mipLevel);
				int32_t cpyPitch = mipSize.x();

				writer->Write<size_t>(mipSize.y() * cpyPitch);

				uint8_t * pSrc = static_cast<uint8_t*>(dataDesc.pData);

				for (int32_t row = 0; row < mipSize.y(); ++row)
				{
					writer->WriteBytes(pSrc, cpyPitch);
					pSrc += dataDesc.rowPitch;
				}

				dumpTex->UnMap();
			}
		}
	}
}