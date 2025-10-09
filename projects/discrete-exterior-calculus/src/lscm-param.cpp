#include "spectral-conformal-parameterization.h"


VertexData<Vector2> SpectralConformalParameterization::arap_flatten() const {
      //0.建立局部坐标系
    //1.求Jacobian矩阵J_t

      //2.求解带符号的SVD 分解,L_t
      
      //2.填充矩阵A,b

    size_t nFaces = mesh->nFaces();
    SparseMatrix<double> A(2*nFaces,2*(nVerteices-2));
    Vector<double> b(2*nFaces);
    std::vector<Eigen::Triplet<double>> A_entries;

    for(face,i in mesh.faces())
    {
      for(j in face)
      {
        //3.cot_thetaij
     


      }
    }
      


}
VertexData<Vector2> SpectralConformalParameterization::lscm_flatten() const {

    VertexData<Vector2> result(*mesh);
    //0.建立局部坐标系
    //1.选择两个pin点，边界上距离最远的两个点
    //2.填充矩阵A,b

    size_t nFaces = mesh->nFaces();
    SparseMatrix<double> A(2*nFaces,2*(nVerteices-2));
    Vector<double> b(2*nFaces);
    std::vector<Eigen::Triplet<double>> A_entries;

    for(face,i in mesh.faces())
    {
      for(j in face)
      {
        Wj_ti = () + i*(yk-yj)
        real =22
        img = 333
        if (j in pin)
        {
          real*dd+img*33
        }
        else{
            //选择4个位置填充A矩阵
            vertex[j]
        }
      }
    }
    //3.求解 x = (ATA)_-1Ab
    //可以用Eigen直接求逆吗？
    for (Vertex v: mesh->vertices())
    {
    
      Vector3 pos = geometry->inputVertexPositions[v];
      result[v].x = pos.x;
      result[v].y = pos.y;
    }
    return result;
}