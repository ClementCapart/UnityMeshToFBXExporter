#include <fbxsdk.h>
#include <fbxsdk/core/fbxsystemunit.h>
#include <map>
#include <vector>

#define DllExport __declspec(dllexport)

struct Color
{
	float r;
	float g;
	float b;
	float a;
};

struct Vector3f
{
	float x;
	float y;
	float z;
};

struct Vector2f
{
	float x;
	float y;
};

void SaveFile(const char* filePath, FbxScene* scene, FbxManager* manager);
void AssignUV(FbxGeometryElementUV* meshUV, Vector2f* uvs, int uvCount, int* triangles, int triangleCount, bool meldDuplicates);

extern "C"
{
	DllExport bool ExportToFBX(const char* exportFilePath, const char* meshName, Vector3f* vertices, int vertCount, bool meldDuplicateVertices, int* triangles, int triangleCount, Vector3f* normals, int normalsCount,
		Vector2f* uv, int uvCount, bool meldUvs, Color* colors, int colorCount, Vector2f* uv2, int uv2Count, Vector2f* uv3, int uv3Count, Vector2f* uv4, int uv4Count);

	bool ExportToFBX(const char* exportFilePath, const char* meshName, Vector3f* vertices, int vertCount, bool meldDuplicateVertices, int* triangles, int triangleCount, Vector3f* normals, int normalsCount, 
		Vector2f* uv, int uvCount, bool meldUvs, Color* colors, int colorCount, Vector2f* uv2, int uv2Count, Vector2f* uv3, int uv3Count, Vector2f* uv4, int uv4Count)
	{
		// Initialize manager and scene
		FbxManager* sdkManager = FbxManager::Create();
		FbxIOSettings* settings = FbxIOSettings::Create(sdkManager, IOSROOT);
		sdkManager->SetIOSettings(settings);
		FbxScene* scene = FbxScene::Create(sdkManager, "");		

		// Change system unit to meters to avoid Laura's wrath (and because it makes sense, too.)
		if (scene->GetGlobalSettings().GetSystemUnit() == FbxSystemUnit::cm)
		{
			FbxSystemUnit::m.ConvertScene(scene);
		}		

		// Get scene root node
		FbxNode* rootNode = scene->GetRootNode();

		// Create base node for one mesh
		FbxNode* meshNode = FbxNode::Create(scene, meshName);
		FbxMesh* mesh = FbxMesh::Create(scene, meshName);
		meshNode->SetNodeAttribute(mesh);		
		rootNode->AddChild(meshNode);
		
		// Remove perfect duplicate vertices from the list
		std::map<int, int> duplicateVertexToIndex;
		std::vector<int> alreadyAddedVertices;

		for (int i = 0; i < vertCount; i++)
		{
			bool foundDuplicate = false;

			if (meldDuplicateVertices)
			{
				for (int j = 0; j < alreadyAddedVertices.size(); j++)
				{
					if (vertices[alreadyAddedVertices[j]].x == vertices[i].x
						&& vertices[alreadyAddedVertices[j]].y == vertices[i].y
						&& vertices[alreadyAddedVertices[j]].z == vertices[i].z)
					{
						foundDuplicate = true;
						duplicateVertexToIndex[i] = j;
						break;
					}
				}
			}

			if (!foundDuplicate)
			{
				duplicateVertexToIndex[i] = alreadyAddedVertices.size();
				alreadyAddedVertices.push_back(i);
			}
		}

		// Add vertices to the mesh list
		mesh->InitControlPoints(alreadyAddedVertices.size());
		FbxVector4* controlPoints = mesh->GetControlPoints();
		for (int i = 0; i < alreadyAddedVertices.size(); i++)
		{
			controlPoints[i] = FbxVector4(vertices[alreadyAddedVertices[i]].x * -1, vertices[alreadyAddedVertices[i]].y, vertices[alreadyAddedVertices[i]].z);
		}

		// Add triangles
		for (int i = 0; i + 2 < triangleCount; i = i+3)
		{
			mesh->BeginPolygon();
			mesh->AddPolygon(duplicateVertexToIndex[triangles[i+0]]);
			mesh->AddPolygon(duplicateVertexToIndex[triangles[i+2]]);
			mesh->AddPolygon(duplicateVertexToIndex[triangles[i+1]]);
			mesh->EndPolygon();
		}
		mesh->BuildMeshEdgeArray();

		// Add normals		
		if (normalsCount != 0)
		{
			FbxGeometryElementNormal* meshNormals = mesh->CreateElementNormal();
			meshNormals->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
			meshNormals->SetReferenceMode(FbxGeometryElement::eDirect);

			for (int i = 0; i < triangleCount; i = i+3)
			{
				meshNormals->GetDirectArray().Add(FbxVector4(normals[triangles[i+0]].x * -1, normals[triangles[i+0]].y, normals[triangles[i+0]].z, 0.0f));
				meshNormals->GetDirectArray().Add(FbxVector4(normals[triangles[i+2]].x * -1, normals[triangles[i+2]].y, normals[triangles[i+2]].z, 0.0));
				meshNormals->GetDirectArray().Add(FbxVector4(normals[triangles[i+1]].x * -1, normals[triangles[i+1]].y, normals[triangles[i+1]].z, 0.0));
			}
		}

		// Add UV1
		if (uvCount != 0)
		{
			FbxGeometryElementUV* meshUV = mesh->CreateElementUV("UV1");
			meshUV->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
			meshUV->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

			AssignUV(meshUV, uv, uvCount, triangles, triangleCount, meldUvs);
		}

		// Add vertex colors
		if (colorCount != 0)
		{
			FbxGeometryElementVertexColor* meshColors = mesh->CreateElementVertexColor();
			meshColors->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
			meshColors->SetReferenceMode(FbxGeometryElement::eDirect);

			for (int i = 0; i < triangleCount; i = i + 3)
			{
				meshColors->GetDirectArray().Add(FbxVector4(colors[triangles[i+0]].r, colors[triangles[i+0]].g, colors[triangles[i+0]].b, colors[triangles[i+0]].a));
				meshColors->GetDirectArray().Add(FbxVector4(colors[triangles[i+2]].r, colors[triangles[i+2]].g, colors[triangles[i+2]].b, colors[triangles[i+2]].a));
				meshColors->GetDirectArray().Add(FbxVector4(colors[triangles[i+1]].r, colors[triangles[i+1]].g, colors[triangles[i+1]].b, colors[triangles[i+1]].a));
			}
		}

		// Add UV2
		if (uv2Count != 0)
		{
			FbxGeometryElementUV* meshUV = mesh->CreateElementUV("UV2");
			meshUV->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
			meshUV->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

			AssignUV(meshUV, uv2, uv2Count, triangles, triangleCount, meldUvs);
		}

		// Add UV3
		if (uv3Count != 0)
		{
			FbxGeometryElementUV* meshUV = mesh->CreateElementUV("UV3");
			meshUV->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
			meshUV->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

			AssignUV(meshUV, uv3, uv3Count, triangles, triangleCount, meldUvs);
		}

		// Add UV4
		if (uv4Count != 0)
		{
			FbxGeometryElementUV* meshUV = mesh->CreateElementUV("UV4");
			meshUV->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
			meshUV->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

			AssignUV(meshUV, uv4, uv4Count, triangles, triangleCount, meldUvs);
		}

		// Export to path and finish
		SaveFile(exportFilePath, scene, sdkManager);
	}
}

void AssignUV(FbxGeometryElementUV* meshUV, Vector2f* uvs, int uvCount, int* triangles, int triangleCount, bool meldDuplicates)
{
	std::map<int, int> duplicateVertexToIndex;
	std::vector<int> uniqueVertices;
	for (int i = 0; i < uvCount; i++)
	{
		bool foundDuplicate = false;

		if (meldDuplicates)
		{
			for (int j = 0; j < uniqueVertices.size(); j++)
			{
				if (uvs[uniqueVertices[j]].x == uvs[i].x
					&& uvs[uniqueVertices[j]].y == uvs[i].y)
				{
					foundDuplicate = true;
					duplicateVertexToIndex[i] = j;
				}
			}
		}

		if (!foundDuplicate)
		{
			duplicateVertexToIndex[i] = uniqueVertices.size();
			uniqueVertices.push_back(i);
		}
	}

	for (int i = 0; i < uniqueVertices.size(); i++)
	{
		meshUV->GetDirectArray().Add(FbxVector2(uvs[uniqueVertices[i]].x, uvs[uniqueVertices[i]].y));
	}

	for (int i = 0; i < triangleCount; i = i + 3)
	{
		meshUV->GetIndexArray().Add(duplicateVertexToIndex[triangles[i+0]]);
		meshUV->GetIndexArray().Add(duplicateVertexToIndex[triangles[i+2]]);
		meshUV->GetIndexArray().Add(duplicateVertexToIndex[triangles[i+1]]);
	}
}

void SaveFile(const char* filePath, FbxScene* scene, FbxManager* manager)
{
	FbxExporter* exporter = FbxExporter::Create(manager, "");
	bool exportStatus = exporter->Initialize(filePath, -1, manager->GetIOSettings());
	if (!exportStatus)
	{
		printf("Exporter Initialization failed\n");
		printf("Error: %s\n\n", exporter->GetStatus().GetErrorString());
		return;
	}

	exporter->Export(scene);
	exporter->Destroy();
}

