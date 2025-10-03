#include "spectral-conformal-parameterization.h"



VertexData<Vector2> SpectralConformalParameterization::lscm_flatten() const {

    VertexData<Vector2> result(*mesh);

    for (Vertex v: mesh->vertices())
    {
    
        Vector3 pos = geometry->inputVertexPositions[v];
      result[v].x = pos.x;
      result[v].y = pos.y;
    }
    return result;
}