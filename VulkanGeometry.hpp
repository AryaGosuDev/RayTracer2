#ifndef __VK_GEOMETRY_HPP__
#define __VK_GEOMETRY_HPP__

#define EPS 1e-6

namespace VkApplication {

	void MainVulkApplication::loadModel() {

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(MODEL_PATH,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return;
		}



		for (size_t i = 0; i < scene->mNumMeshes; ++i) {

			aiMesh* mesh = scene->mMeshes[i];

			for (size_t j = 0; j < mesh->mNumVertices; ++j) {
				Vertex vertex = {};

				vertex.pos = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };

				if (mesh->HasNormals()) 
					vertex.normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

				if (mesh->HasTextureCoords(0))
					vertex.texCoord = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };
				else vertex.texCoord = { 0.0f, 0.0f };

				vertices.push_back(vertex);
			}

			for (size_t j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				for (size_t k = 0; k < face.mNumIndices; k++) {
					indices.push_back(face.mIndices[k]);
				}
			}



		}
	}
}
#endif