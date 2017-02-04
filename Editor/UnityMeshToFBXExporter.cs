using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

#if UNITY_EDITOR
using UnityEditor;

public class MeshFBXExporter
{
	[DllImport("FBXExporter")]
	private static extern bool ExportFBX([In] string filePath, [In] string meshName, [In] Vector3[] verts, [In] int vertCount, [In] bool meldDuplicateVertices, [In] int[] triangles, [In] int triangleCount, [In] Vector3[] normals, int normalCount, [In] Vector2[] uv = null, int uvCount = 0, [In] bool meldDuplicateUVs = true, [In] Color[] colors = null, int colorCount = 0, [In] Vector2[] uv2 = null, int uv2Count = 0, [In] Vector2[] uv3 = null, int uv3Count = 0, [In] Vector2[] uv4 = null, int uv4Count = 0);

	private static void InternalExportMesh(string filePath, string meshName, Mesh mesh, bool meldDuplicateVertices = true, bool meldDuplicateUVs = true)
	{
		if (filePath != "" && mesh != null)
		{
			ExportFBX(filePath, meshName, mesh.vertices, mesh.vertexCount, meldDuplicateVertices, mesh.triangles, mesh.triangles.Length, mesh.normals, mesh.normals.Length, mesh.uv, mesh.uv.Length, meldDuplicateUVs, mesh.colors, mesh.colors.Length, mesh.uv2, mesh.uv2.Length, mesh.uv3, mesh.uv3.Length, mesh.uv4, mesh.uv4.Length);
		}
	}

	public static void ExportMesh(string filePath, string meshName, Mesh mesh, bool meldDuplicateVertices = true, bool meldDuplicateUVs = true)
	{
		InternalExportMesh(filePath, meshName, mesh, meldDuplicateVertices, meldDuplicateUVs);
	}	
}
#endif
