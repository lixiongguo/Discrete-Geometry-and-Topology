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
#include "spectral-conformal-parameterization.h"
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
std::string currentFilePath = "../../../input/Nefertiti_face.obj";
bool showFileDialog = false;
char filePathBuffer[256];

// 展平算法相关变量
enum class FlattenMethod { Tutte, ARAP, LSCM, SCP};
FlattenMethod currentFlattenMethod = FlattenMethod::Tutte;
polyscope::SurfaceParameterizationQuantity* checkerboard;
Vector3 CoM;                        // original center of mass, for re-centering purposes
VertexData<Vector3> ORIGINAL;       // original vertex positions
VertexData<Vector3> SCP_MESH;       // vertex positions of SCP
VertexData<Vector2> SCP_FLATTENING; // SCP solution
bool DISPLAY_FLAT = false;

// Windows文件选择对话框函数
#ifdef _WIN32
std::string openFileBrowser() {
    OPENFILENAME ofn;
    char fileName[256] = "";
    char initialDir[MAX_PATH] = "../../../input/"; // 设置默认目录
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = sizeof(fileName);
    ofn.lpstrTitle = "Select Model File";
    ofn.lpstrInitialDir = initialDir; // 设置初始目录
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        return std::string(fileName);
    }
    
    return "";
}
#endif
#ifdef __APPLE__
#include <cstdio>
#include <iostream>
#include <string>

std::string openFileBrowser() {
    // 使用AppleScript调用macOS文件选择对话框
    std::string command = "osascript -e 'tell application \"Finder\" to set selectedFile to choose file with prompt \"选择OBJ模型文件\" of type {\"obj\"}' -e 'get POSIX path of selectedFile' 2>/dev/null";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";

    char buffer[1024];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // 移除换行符
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
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

void addCheckerboard(VertexData<Vector2>& flattening) {

    std::vector<std::array<double, 2>> V_uv(mesh->nVertices());
    for (Vertex v : mesh->vertices()) {
        V_uv[v.getIndex()] = {flattening[v][0], flattening[v][1]};
    }
    checkerboard = psMesh->addVertexParameterizationQuantity("checkerboard", V_uv);
    checkerboard->setEnabled(true);
    checkerboard->setStyle(polyscope::ParamVizStyle::CHECKER);
    checkerboard->setCheckerColors(std::make_pair(glm::vec3{1.0, 0.45, 0.0}, glm::vec3{0.55, 0.27, 0.07}));
    checkerboard->setCheckerSize(0.002);
}
VertexData<Vector3> mapToMeshData(VertexData<Vector2>& flattening) {

    VertexData<Vector3> flat = geometry->inputVertexPositions;
    for (Vertex v : mesh->vertices()) {
        flat[v] = Vector3{flattening[v][0], flattening[v][1], 0.0};
    }
    return flat;
}
// 展平算法函数实现（占位符）
void flattenMesh(FlattenMethod method) {
    // TODO: 实现具体的展平算法
    switch (method) {
        case FlattenMethod::Tutte:
            // 实现 Tutte 展平算法
            break;
        case FlattenMethod::ARAP:
            // 实现 ARAP 展平算法
            break;
        case FlattenMethod::LSCM:
            // 实现 LSCM 展平算法
            break;
    }
    ORIGINAL = geometry->inputVertexPositions;
    SpectralConformalParameterization SCP = SpectralConformalParameterization(mesh.get(), geometry.get());
    SCP_FLATTENING = SCP.flatten();
    SCP_MESH = mapToMeshData(SCP_FLATTENING);
    addCheckerboard(SCP_FLATTENING);

    if (DISPLAY_FLAT) 
    {
        // Display SCP parameterization
        geometry->inputVertexPositions = SCP_MESH;
        geometry->normalize(CoM, true);
        polyscope::view::flyToHomeView();
        polyscope::view::style = polyscope::view::NavigateStyle::Planar;
    } else {
        // Display original
        geometry->inputVertexPositions = ORIGINAL;
        polyscope::view::style = polyscope::view::NavigateStyle::Turntable;
    }
    redraw();
}

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
#elif __APPLE__
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
        // 展平算法下拉列表
    const char* flattenMethodNames[] = { "Tutte", "ARAP", "LSCM","SCP" };
    int currentMethodIndex = static_cast<int>(currentFlattenMethod);
    if (ImGui::Combo("Flatten Method", &currentMethodIndex, flattenMethodNames, IM_ARRAYSIZE(flattenMethodNames))) {
        currentFlattenMethod = static_cast<FlattenMethod>(currentMethodIndex);
    }
    
    // 展平按钮
    if (ImGui::Checkbox("Flat", &DISPLAY_FLAT)) {
        flattenMesh(currentFlattenMethod);
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