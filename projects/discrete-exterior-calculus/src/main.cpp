// DISCRETE EXTERIOR CALCULUS

#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/vertex_position_geometry.h"

#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

#include "args/args.hxx"
#include "imgui.h"


#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#endif


#include "colormap.h"
#include "solvers.h"
#include <algorithm>
#include <map>

// ** taken from Polyscope gl_engine.h -- not sure how to avoid using OpenGL for texture loading
#include "stb_image.h"
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"
#else
#include "glad/glad.h"
// glad must come first
#include "GLFW/glfw3.h"
#endif

using namespace geometrycentral;
using namespace geometrycentral::surface;

std::unique_ptr<ManifoldSurfaceMesh> mesh;
std::unique_ptr<VertexPositionGeometry> geometry;

polyscope::SurfaceMesh* psMesh;
std::string currentFilePath = "../../../input/cowhead.obj";
bool showFileDialog = false;
char filePathBuffer[256];


// Windows文件选择对话框函数
#ifdef _WIN32
std::string openFileBrowser() {
    OPENFILENAME ofn;
    char fileName[256] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = sizeof(fileName);
    ofn.lpstrTitle = "Select Model File";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        return std::string(fileName);
    }
    
    return "";
}
#endif


void flipZ() {
    // Rotate mesh 180 deg about up-axis on startup
    glm::mat4x4 rot = glm::rotate(glm::mat4x4(1.0f), static_cast<float>(PI), glm::vec3(0, 1, 0));
    for (Vertex v : mesh->vertices()) {
        Vector3 vec = geometry->inputVertexPositions[v];
        glm::vec4 rvec = {vec[0], vec[1], vec[2], 1.0};
        rvec = rot * rvec;
        geometry->inputVertexPositions[v] = {rvec[0], rvec[1], rvec[2]};
    }
    psMesh->updateVertexPositions(geometry->inputVertexPositions);
}

void showSelected() {
    // pass
}


void redraw() {
    polyscope::requestRedraw();
}

void loadMesh(const std::string& filepath) {
    // Unregister the previous mesh if it exists
    if (psMesh) {
        polyscope::removeStructure(psMesh);
    }
    
    std::tie(mesh, geometry) = readManifoldSurfaceMesh(filepath);
    psMesh = polyscope::registerSurfaceMesh("Primal mesh", geometry->inputVertexPositions, mesh->getFaceVertexList(),
                                            polyscopePermutations(*mesh));
    psMesh->setEnabled(true);
    
    currentFilePath = filepath;
    strcpy(filePathBuffer, filepath.c_str());
    flipZ();
}
/*
 * User-defined buttons
 */
void functionCallback() {
    if (ImGui::Button("Select Model File")) {
#ifdef _WIN32
        std::string selectedFile = openFileBrowser();
        if (!selectedFile.empty()) {
            try {
                ::loadMesh(selectedFile);
                currentFilePath = selectedFile;
                strcpy(filePathBuffer, currentFilePath.c_str());
            } catch (const std::exception& e) {
                // Handle error
            }
        }
#else
        showFileDialog = true;
#endif
    }
    
    if (showFileDialog) {
        ImGui::OpenPopup("Select Model File");
        showFileDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Select Model File", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Current file: %s", currentFilePath.c_str());
        ImGui::InputText("File Path", filePathBuffer, IM_ARRAYSIZE(filePathBuffer));
        
        if (ImGui::Button("Load")) {
            try {
                ::loadMesh(std::string(filePathBuffer));
                ImGui::CloseCurrentPopup();
            } catch (const std::exception& e) {
                // Handle error - in a real application you might want to show this to the user
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

}


int main(int argc, char** argv) {

    strcpy(filePathBuffer, currentFilePath.c_str());
    ::loadMesh(currentFilePath);

    polyscope::init();
    polyscope::state::userCallback = functionCallback;
    polyscope::show();

    return EXIT_SUCCESS;
}