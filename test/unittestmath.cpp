#include <gtest/gtest.h>
#include "math/algebra.h"

TEST(Vector3DTest, Creation)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	EXPECT_NEAR(1, vec1.x, 1e-08);
	EXPECT_NEAR(2, vec1.y, 1e-08);
	EXPECT_NEAR(3, vec1.z, 1e-08);
	Vector3D vec2;
	EXPECT_NEAR(0, vec2.x, 1e-08);
	EXPECT_NEAR(0, vec2.y, 1e-08);
	EXPECT_NEAR(0, vec2.z, 1e-08);
}

TEST(Vector3DTest, Length)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	EXPECT_NEAR(3.741657386773941, vec1.Length(), 1e-08);
	Vector3D vec2(3,2,1);
	EXPECT_NEAR(vec1.Length(), vec2.Length(), 1e-08);
}

TEST(Vector3DTest, Nomalize)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	vec1.Normalise();
	EXPECT_NEAR(1, vec1.Length(), 1e-8);
	EXPECT_NEAR(0.267261241912424, vec1.x, 1e-8);
	EXPECT_NEAR(0.534522483824849, vec1.y, 1e-8);
	EXPECT_NEAR(0.8017837257372730, vec1.z, 1e-8);
}

TEST(Vector3DTest, Equality)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	Vector3D vec2(1,2,3);
	EXPECT_EQ(vec1, vec2);
	Vector3D vec3(2,2,3);
	EXPECT_FALSE(vec1 == vec3);
	Vector3D vec4(2,3,4);
	EXPECT_TRUE(vec1 < vec4);
	EXPECT_TRUE(vec4 > vec1);
	EXPECT_FALSE(vec1 < vec3);
}

TEST(Vector3DTest, Muliplication)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	Vector3D vec2(2,4,6);
	EXPECT_EQ(vec1*2, vec2);
	EXPECT_EQ(2*vec1, vec2);
	EXPECT_NEAR(14, vec1*vec1, 1e-08);
	EXPECT_NEAR(28, vec1*vec2, 1e-08);
	Vector3D vec3;
	vec3.CrossProduct(vec1,vec2);
	EXPECT_NEAR(0, vec3.x, 1e-08);
	EXPECT_NEAR(0, vec3.y, 1e-08);
	EXPECT_NEAR(0, vec3.z, 1e-08);
	vec3.CrossProduct(vec1,Vector3D(3,2,1));
	EXPECT_NEAR(-4, vec3.x, 1e-08);
	EXPECT_NEAR(8, vec3.y, 1e-08);
	EXPECT_NEAR(-4, vec3.z, 1e-08);
	
	Vector3D vec4 = CrossProduct(vec1,Vector3D(3,2,1));
	EXPECT_NEAR(-4, vec4.x, 1e-08);
	EXPECT_NEAR(8, vec4.y, 1e-08);
	EXPECT_NEAR(-4, vec4.z, 1e-08);
}

TEST(Vector3DTest, Addition)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	Vector3D vec2(0,9,8);
	EXPECT_EQ(Vector3D(1,11,11), vec1+vec2);
}

TEST(Vector3DTest, Subtraction)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	Vector3D vec2(0,9,8);
	EXPECT_EQ(Vector3D(1,-7,-5), vec1-vec2);
}

TEST(Vector3DTest, Point3D)
{
	using cap::Point3D;
	using cap::Vector3D;
	
	Point3D point1(2,4,5);
	Point3D point2(1,2,2);
	Vector3D vec1(1,2,3);
	EXPECT_EQ(point1-point2, vec1);
	Point3D point3 = point2 + vec1;
	EXPECT_NEAR(point3.x, point1.x, 1e-08);
	EXPECT_NEAR(point3.y, point1.y, 1e-08);
	EXPECT_NEAR(point3.z, point1.z, 1e-08);
}

TEST(Matrix3x3Test, Init)
{
    using cap::Matrix3x3;
    Matrix3x3 mx;
    EXPECT_EQ(0.0, mx(1,2));
    Matrix3x3 id;// = Matrix3x3::Identity();
    EXPECT_EQ(1.0, id(1,1));
    EXPECT_EQ(0.0, id(2,1));
    Matrix3x3 as = id;
    EXPECT_EQ(1.0, as(1,1));
    EXPECT_EQ(1.0, as(3,3));

}

TEST(Matrix3x3Test, ScalarMultiply)
{
    using cap::Matrix3x3;
    Matrix3x3 mx;// = Matrix3x3::Identity();
    Matrix3x3 mxA = mx * 3;
    EXPECT_EQ(3.0, mxA(1,1));
    Matrix3x3 mxB = 3 * mx;
    EXPECT_EQ(3.0, mxB(2,2));
}

TEST(Matrix3x3Test, VectorMultiply)
{
    using cap::Matrix3x3;
    using cap::Vector3D;
    Matrix3x3 mx;// = Matrix3x3::Identity();
    Vector3D vc(1.0, 2.0, 3.0);
    Vector3D vcA = mx * vc;
    EXPECT_EQ(1.0, vcA(1));
    EXPECT_EQ(2.0, vcA(2));
    EXPECT_EQ(3.0, vcA(3));
    Vector3D c1(1.0, 4.0, 7.0);
    Vector3D c2(2.0, 5.0, 8.0);
    Vector3D c3(3.0, 6.0, 9.0);
    Matrix3x3 mxA(c1, c2, c3);
    Vector3D vcB = mxA*vc;
    EXPECT_EQ(14.0, vcB(1));
    EXPECT_EQ(32.0, vcB(2));
    EXPECT_EQ(50.0, vcB(3));
    Vector3D vcC = vc*mxA;
    EXPECT_EQ(30.0, vcC(1));
    EXPECT_EQ(36.0, vcC(2));
    EXPECT_EQ(42.0, vcC(3));
}

TEST(Matrix3x3Test, Functions)
{
    using cap::Matrix3x3;
    Matrix3x3 mx(1, 2, 3, 4, 5, 6, 7, 8, 9);
    mx.Transpose();
    EXPECT_EQ(2, mx(2,1));
    EXPECT_EQ(8, mx(2,3));
    Matrix3x3 mxI(1, 4, 7, 2, 6, 8, 3, 4, 9);
    mxI.Invert();
    EXPECT_NEAR(-0.916667, mxI[0], 1e-5);
    EXPECT_NEAR(-0.25, mxI[5], 1e-5);
}

TEST(Matrix3x3Test, TransformsBasic)
{
    using cap::Matrix3x3;
    using cap::Vector3D;
    Vector3D v1( 0, 1, 0);
    Vector3D v2(-1, 0, 0);
    Vector3D v3( 0, 0, 1);

    Vector3D del(-2, 0, 0);

    Vector3D delB;
    delB.x = v1*del;
    delB.y = v2*del;
    delB.z = v3*del;

    EXPECT_EQ(0, delB.x);
    EXPECT_EQ(2, delB.y);
    EXPECT_EQ(0, delB.z);
}

TEST(Matrix3x3Test, Transforms1)
{
    using cap::Matrix3x3;
    using cap::Vector3D;
    Vector3D v1( 0.5, 2, 1);
    Vector3D v2(-0.5, 2, 0);
    Vector3D v3( 0.5, 0, 1);

    Vector3D axis1 = v2 - v1;
    axis1.Normalise();
    Vector3D axis2 = v3 - v1;
    axis2.Normalise();
    Vector3D axis3 = CrossProduct(axis1, axis2);
    axis3.Normalise();

    EXPECT_NEAR(-0.70710, axis3.x, 1e-4);
    EXPECT_NEAR( 0, axis3.y, 1e-4);
    EXPECT_NEAR( 0.7071, axis3.z, 1e-4);

    // init pos [462, 388] -> [-249.38398051994074, 225.30415661121398, 266.41129318957701]


}

TEST(Matrix3x3Test, Transforms2)
{
    using cap::Matrix3x3;
    using cap::Vector3D;
    Vector3D v1(  88.6768, 86.5491, -147.945);
    Vector3D v2(-104.26, -111.161,  -147.945);
    Vector3D v3( 220.85,  -42.4337,  137.526);

    Vector3D axis1 = v2 - v1;
    axis1.Normalise();
    Vector3D axis2 = v3 - v1;
    axis2.Normalise();
    Vector3D axis3 = CrossProduct(axis1, axis2);
    axis3.Normalise();

    EXPECT_NEAR(-0.600910, axis3.x, 1e-4);
    EXPECT_NEAR( 0.586403, axis3.y, 1e-4);
    EXPECT_NEAR( 0.543174, axis3.z, 1e-4);

    // init pos [462, 388] -> [-249.38398051994074, 225.30415661121398, 266.41129318957701]


}

TEST(Matrix3x3Test, Transforms3)
{
    using cap::Matrix3x3;
    using cap::Vector3D;
    Vector3D blc(-109.01411274960938, -102.26087572343749, -147.91112250374732);
//    Vector3D brc(83.923105587890632, 95.449246464062497, -147.91112793235845);
    Vector3D tlc(23.159527250390628, -231.2437143234375, 137.55964009625268);
    Vector3D trc(216.0967455878, -33.533596346676, 137.55963446676);
    Vector3D delta(0.347168, -0.338787, 0.74982);

    Vector3D axis1 = trc - tlc;
    axis1.Normalise();
    Vector3D axis2 = blc - tlc;
    axis2.Normalise();
    Vector3D axis3 = CrossProduct(axis1, axis2);
    axis3.Normalise();
    Matrix3x3 rot(axis1, axis2, axis3);
    Vector3D r = rot*delta;

    EXPECT_NEAR(-0.60090985335800506, axis3.x, 1e-8);
    EXPECT_NEAR( 0.58640336580982466, axis3.y, 1e-8);
    EXPECT_NEAR( 0.54317441094382368, axis3.z, 1e-8);

    EXPECT_NEAR(0, DotProduct(r, axis3), 1e-6);

    // init pos [462, 388] -> [-249.38398051994074, 225.30415661121398, 266.41129318957701]
}

TEST(HomogeneousVectorTest, Init)
{
    using cap::HomogeneousVector3D;
    HomogeneousVector3D h1;
    EXPECT_EQ(0.0, h1(2));
    EXPECT_EQ(1.0, h1(4));
    cap::Vector3D vc(5.0, 4.0, 3.0);
    HomogeneousVector3D h2(vc);
    EXPECT_EQ(5.0, h2(1));
    EXPECT_EQ(3.0, h2(3));
}

TEST(Matrix4x4Test, Init)
{
    using cap::Matrix4x4;
    Matrix4x4 mx;
    EXPECT_EQ(0.0, mx(2,3));
    EXPECT_EQ(0.0, mx(4,1));
    Matrix4x4 mx1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    EXPECT_EQ(6, mx1(2,2));
    EXPECT_EQ(4, mx1(1,4));
    EXPECT_EQ(15, mx1(4,3));
}

TEST(Matrix4x4Test, Functions)
{
    using cap::Matrix4x4;
    cap::Matrix3x3 rot(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix4x4 mx;
    mx.SetRotation(rot);
    EXPECT_EQ(5, mx(2,2));
    EXPECT_EQ(3, mx(1,3));
    EXPECT_EQ(8, mx(3,2));

    cap::Vector3D vc(29, 28, 27);
    mx.SetTranslation(vc);
    EXPECT_EQ(28, mx(2,4));

    Matrix4x4 pr(1.0/300, 0.0, 0.0, 0.0,
                 0.0, 1.0/300, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4172.74-11.6764), -(4172.74+11.6764)/(4172.74-11.6764),
                 0.0, 0.0, 0.0, 1.0);
    Matrix4x4 prinv(1.0/300, 0.0, 0.0, 0.0,
                 0.0, 1.0/300, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4172.74-11.6764), -(4172.74+11.6764)/(4172.74-11.6764),
                 0.0, 0.0, 0.0, 1.0);
    prinv.InvertAffine();
    Matrix4x4 id = pr*prinv;

    EXPECT_NEAR(1.0, id[0], 1e-8);
    EXPECT_NEAR(1.0, id[5], 1e-8);
    EXPECT_NEAR(1.0, id[10], 1e-8);
    EXPECT_NEAR(1.0, id[15], 1e-8);
    EXPECT_NEAR(0.0, id[3], 1e-8);
    EXPECT_NEAR(0.0, id[7], 1e-8);
    EXPECT_NEAR(0.0, id[11], 1e-8);
    EXPECT_NEAR(0.0, id[13], 1e-8);

    cap::Vector3D eye(-249.383, 225.304, 266.411);
    cap::Vector3D lookat(53.5413, -67.8972, -5.17574);
    cap::Vector3D up(-0.38759008819937724, 0.37824264811011848, -0.84065832695587084);
    cap::Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    cap::Vector3D xaxis = CrossProduct(up, zaxis);
    xaxis.Normalise();
    cap::Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();
    cap::Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    cap::Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));

    Matrix4x4 mv, mvinv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mvinv.SetRotation(mvrot);
    mvinv.SetTranslation(mvtr);
    mvinv.InvertEuclidean();

    Matrix4x4 id2 = mv*mvinv;
    EXPECT_NEAR(1.0, id2[0], 1e-8);
    EXPECT_NEAR(1.0, id2[5], 1e-8);
    EXPECT_NEAR(1.0, id2[10], 1e-8);
    EXPECT_NEAR(1.0, id2[15], 1e-8);
    EXPECT_NEAR(0.0, id2[3], 1e-8);
    EXPECT_NEAR(0.0, id2[7], 1e-8);
    EXPECT_NEAR(0.0, id2[11], 1e-8);
    EXPECT_NEAR(0.0, id2[13], 1e-8);


//    dbg(ToString(mx));

}

TEST(TransformTest, ObjectToWindow)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::HomogeneousVector3D;
    using cap::Matrix4x4;
    Vector3D eye(-249.383, 225.304, 266.411);
    Vector3D lookat(53.5413, -67.8972, -5.17574);
    Vector3D up(-0.38759008819937724, 0.37824264811011848, -0.84065832695587084);
    Vector3D brc(82.0198, 94.9634, -149.492);
//    Vector3D blc(-110.917, -102.747, -149.492);
//    Vector3D trc(214.193, -34.0194, 135.979);
//    Vector3D tlc(21.2563, -231.73, 135.979);
    HomogeneousVector3D hbrc(brc);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    xaxis.Normalise();
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();
    EXPECT_NEAR(1.0, up.Length(), 1e-12);
    EXPECT_NEAR(1.0, yaxis.Length(), 1e-12);
    EXPECT_NEAR(up.x, yaxis.x, 1e-12);
    EXPECT_NEAR(up.y, yaxis.y, 1e-12);
    EXPECT_NEAR(up.z, yaxis.z, 1e-12);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));

    EXPECT_NEAR(eye.Length(), mvtr.Length(), 1e-06);
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    EXPECT_EQ(xaxis.x, mv(1,1));
    EXPECT_EQ(zaxis.y, mv(3,2));
    EXPECT_EQ(xaxis.z, mv(1,3));
    EXPECT_EQ(0.0, mv(4,1));

    HomogeneousVector3D heye = mv * hbrc;
    EXPECT_NEAR(-136.4471459, heye.x, 1e-08);
    EXPECT_NEAR(171.883505975, heye.y, 1e-08);
//    EXPECT_EQ(447.825, heye.z);
//    EXPECT_EQ(1.0, heye.w);

    Matrix4x4 pr(1.0/300, 0.0, 0.0, 0.0,
                 0.0, 1.0/300, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4172.74-11.6764), -(4172.74+11.6764)/(4172.74-11.6764),
                 0.0, 0.0, 0.0, 1.0);
    HomogeneousVector3D hclip = pr * heye;
    EXPECT_NEAR(-0.45482382, hclip.x, 1e-08);
    EXPECT_NEAR(0.57294502, hclip.y, 1e-08);
//    EXPECT_EQ(-1.22086, hclip.z);
    EXPECT_EQ(1.0, hclip.w);

//    EXPECT_NEAR(549, 700.0/2.0*hclip.x+700.0/2.0, 20);
//    EXPECT_NEAR(579, 670.0/2.0*hclip.y+670.0/2.0, 20);
    EXPECT_NEAR(194, 700.0/2.0*hclip.x+700.0/2.0, 10);
    EXPECT_NEAR(522, 672.0/2.0*hclip.y+672.0/2.0, 10);

}

TEST(TransformTest, ObjectToWindowBasic)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    Vector3D eye(-3, 1, 0.5);
    Vector3D lookat(0, 1, 0.5);
    Vector3D up(0, 0, 1);
    Vector3D tlc(0, 2, 1);
    Vector3D trc(0, 0, 1);
    HomogeneousVector3D htlc(tlc);
    HomogeneousVector3D htrc(trc);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_EQ(1.0, yaxis.z);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);

    HomogeneousVector3D htlceye = mv * htlc;
    EXPECT_EQ(1, htlceye.x);
    EXPECT_EQ(0.5, htlceye.y);
    EXPECT_EQ(3, htlceye.z);
    EXPECT_EQ(1.0, htlceye.w);

    HomogeneousVector3D htrceye = mv * htrc;
    EXPECT_EQ(-1, htrceye.x);
    EXPECT_EQ(0.5, htrceye.y);
    EXPECT_EQ(3, htrceye.z);
    EXPECT_EQ(1.0, htrceye.w);

    Matrix4x4 pr(1.0/2, 0.0, 0.0, 0.0,
                 0.0, 1.0/1.0, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4-2), -(4+2)/(4-2),
                 0.0, 0.0, 0.0, 1.0);
    HomogeneousVector3D hclip = pr * htlceye;
    EXPECT_NEAR(0.5, hclip.x, 1e-6);

    EXPECT_NEAR(300, 400.0/2.0*hclip.x+400.0/2.0, 1e-8);
    EXPECT_NEAR(225, 300.0/2.0*hclip.y+300.0/2.0, 1e-8);

}

TEST(TransformTest, MovementDown)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 400, height = 300;
    Vector3D p1(300, 225, 0);
    Vector3D p2(300, 200, 0);
    Vector3D eye(-3, 1, 0.5);
    Vector3D lookat(0, 1, 0.5);
    Vector3D up(0, 0, 1);
//    Vector3D tlc(0, 2, 1);
//    HomogeneousVector3D /*htlc*/(tlc);

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_EQ(0.5, hclip1.x);
    EXPECT_NEAR(0.5, hclip1.y, 1e-6);

    Matrix4x4 pr(1.0/2, 0.0, 0.0, 0.0,
                 0.0, 1.0/1.0, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4-2), -(4+2)/(4-2),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(0.5, hdeltaeye1.y, 1e-4);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_EQ(1.0, yaxis.z);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_EQ(-6, htlc1.x);
    EXPECT_EQ(2, htlc1.y);
    EXPECT_EQ(1.0, htlc1.z);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_EQ(0.0, delta.x);
    EXPECT_EQ(0.0, delta.y);
    EXPECT_NEAR(-0.1666667, delta.z, 1e-6);

    Vector3D normal(1, 0, 0);
    EXPECT_EQ(0, DotProduct(normal, delta));
}

TEST(TransformTest, MovementLeft)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 400, height = 300;
    Vector3D p1(300, 225, 0);
    Vector3D p2(270, 225, 0);
    Vector3D eye(-3, 1, 0.5);
    Vector3D lookat(0, 1, 0.5);
    Vector3D up(0, 0, 1);
//    HomogeneousVector3D /*htlc*/(tlc);

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_EQ(0.5, hclip1.x);
    EXPECT_NEAR(0.5, hclip1.y, 1e-6);

    Matrix4x4 pr(1.0/2, 0.0, 0.0, 0.0,
                 0.0, 1.0/1.0, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4-2), -(4+2)/(4-2),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(0.5, hdeltaeye1.y, 1e-4);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_EQ(1.0, yaxis.z);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_EQ(-6, htlc1.x);
    EXPECT_EQ(2, htlc1.y);
    EXPECT_EQ(1.0, htlc1.z);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_EQ(0.0, delta.x);
    EXPECT_NEAR(-0.3, delta.y, 1e-6);
    EXPECT_NEAR(0.0, delta.z, 1e-6);

//    HomogeneousVector3D hclip = pr * htlceye;
//    EXPECT_NEAR(0.5, hclip.x, 1e-6);

//    EXPECT_NEAR(300, 400.0/2.0*hclip.x+400.0/2.0, 1e-8);
//    EXPECT_NEAR(225, 300.0/2.0*hclip.y+300.0/2.0, 1e-8);
}

TEST(TransformTest, MovementUpRight)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 400, height = 300;
    Vector3D p1(300, 175, 0);
    Vector3D p2(310, 200, 0);
    Vector3D eye(-3, 1, 0.5);
    Vector3D lookat(0, 1, 0.5);
    Vector3D up(0, 0, 1);
//    Vector3D tlc(0, 2, 1);
//    HomogeneousVector3D /*htlc*/(tlc);

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_EQ(0.5, hclip1.x);
    EXPECT_NEAR(0.1666667, hclip1.y, 1e-6);

    Matrix4x4 pr(1.0/2, 0.0, 0.0, 0.0,
                 0.0, 1.0/1.0, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4-2), -(4+2)/(4-2),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(0.166667, hdeltaeye1.y, 1e-4);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_EQ(1.0, yaxis.z);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_EQ(-6, htlc1.x);
    EXPECT_EQ(2, htlc1.y);
    EXPECT_NEAR(0.666667, htlc1.z, 1e-6);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_EQ(0.0, delta.x);
    EXPECT_NEAR(0.1, delta.y, 1e-6);
    EXPECT_NEAR(0.1666667, delta.z, 1e-6);

    Vector3D normal(1, 0, 0);
    EXPECT_EQ(0, DotProduct(normal, delta));
}

TEST(TransformTest, MovementInPlaneBasic)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 400, height = 300;
    Vector3D p1(300, 175, 0);
    Vector3D p2(310, 200, 0);
    Vector3D eye(-3, 1, 3.5);
    Vector3D lookat(0, 1, 0.5);
    Vector3D up(1/2.0*sqrt(2.0), 0, 1/2.0*sqrt(2.0));

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_EQ(0.5, hclip1.x);
    EXPECT_NEAR(0.1666667, hclip1.y, 1e-6);

    Matrix4x4 pr(1.0/2, 0.0, 0.0, 0.0,
                 0.0, 1.0/1.0, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4-2), -(4+2)/(4-2),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(0.166667, hdeltaeye1.y, 1e-4);

    Vector3D zaxis = lookat - eye;
    EXPECT_NEAR(4.2426406871192848, zaxis.Length(), 1e-8);
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    xaxis.Normalise();
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_EQ(1/2.0*sqrt(2.0), yaxis.z);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_NEAR(-5.00347, htlc1.x, 1e-6);
    EXPECT_EQ(2, htlc1.y);
    EXPECT_NEAR(5.7391714, htlc1.z, 1e-6);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_NEAR(0.11785113019, delta.x, 1e-6);
    EXPECT_NEAR(0.1, delta.y, 1e-6);
    EXPECT_NEAR(0.11785113019, delta.z, 1e-6);

    Vector3D normal(1/2.0*sqrt(2.0), 0, -1/2.0*sqrt(2.0));
    EXPECT_EQ(0, DotProduct(normal, delta));
}

TEST(TransformTest, MovementInPlane)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 700, height = 673;
    Vector3D p1(300, 175, 0);
    Vector3D p2(310, 200, 0);
//    Vector3D eye(-249.382749654219, 225.304445635433, 266.411461553859);
    Vector3D eye(-246.913613519029, 225.304445635433, 266.411461553859);
    Vector3D lookat(53.5413164191406, -67.8972339296875, -5.17574391805289);
//    Vector3D up(-0.38759053286585, 0.378242913394051, -0.840658002578901);
    Vector3D up(-0.38874599989308, 0.379361289895661, -0.839619889769073);

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_NEAR(-0.1428571, hclip1.x, 1e-6);
    EXPECT_NEAR(-0.4799405, hclip1.y, 1e-6);

    Matrix4x4 pr(1.0/300.510255242542, 0.0, 0.0, 0.0,
                 0.0, 1.0/300.510255242542, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4172.74110796008-11.6763855553467), -(4172.74110796008+11.6763855553467)/(4172.74110796008-11.6763855553467),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(-144.22706, hdeltaeye1.y, 1e-4);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    xaxis.Normalise();
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_NEAR(-0.83961988975102209, yaxis.z, 1e-10);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_NEAR(-1418.0917921381867, htlc1.x, 1e-6);
    EXPECT_NEAR( 1428.1932344627478, htlc1.y, 1e-6);
    EXPECT_NEAR( 1523.9416238052781, htlc1.z, 1e-6);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_NEAR(-14.675808076134899, delta.x, 1e-6);
    EXPECT_NEAR(  2.324741678964755, delta.y, 1e-6);
    EXPECT_NEAR(-18.745496656524665, delta.z, 1e-6);

    Vector3D normal(-0.60090985987633827, 0.58640335913024033, 0.5431744109438239);
    EXPECT_NEAR(0, DotProduct(normal, delta), 1e-12);
    // Lookat and eye location not looking directly at the plane, we have a little error here
    EXPECT_NEAR(-1.0, DotProduct(normal, zaxis), 1e-8);
    EXPECT_NEAR(0, DotProduct(zaxis, delta), 1e-12);
}

TEST(TransformTest, MovementInPlaneDown)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 700, height = 673;
    Vector3D p1(458, 261, 0);
    Vector3D p2(457, 261, 0);
//    Vector3D eye(-249.382749654219, 225.304445635433, 266.411461553859);
    Vector3D eye(-246.913613519029, 225.304445635433, 266.411461553859);
    Vector3D lookat(53.5413164191406, -67.8972339296875, -5.17574391805289);
//    Vector3D up(-0.38759053286585, 0.378242913394051, -0.840658002578901);
    Vector3D up(-0.38874599989308, 0.379361289895661, -0.839619889769073);

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_NEAR( 0.30857142857142855, hclip1.x, 1e-8);
    EXPECT_NEAR(-0.22436849925705796, hclip1.y, 1e-8);

    Matrix4x4 pr(1.0/300.510255242542, 0.0, 0.0, 0.0,
                 0.0, 1.0/300.510255242542, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4172.74110796008-11.6763855553467), -(4172.74110796008+11.6763855553467)/(4172.74110796008-11.6763855553467),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(-67.425034980124579, hdeltaeye1.y, 1e-4);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    xaxis.Normalise();
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_NEAR(-0.83961988975102209, yaxis.z, 1e-10);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_NEAR(-1542.6945305209447, htlc1.x, 1e-6);
    EXPECT_NEAR( 1360.2388482676388, htlc1.y, 1e-6);
    EXPECT_NEAR( 1459.4571174963678, htlc1.z, 1e-6);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_NEAR(0.5996598592303144, delta.x, 1e-6);
    EXPECT_NEAR(  0.61449431691539758, delta.y, 1e-6);
    EXPECT_NEAR(-1.771468305378221e-08, delta.z, 1e-6);

    Vector3D normal(-0.60090985987633827, 0.58640335913024033, 0.5431744109438239);
    EXPECT_NEAR(0, DotProduct(normal, delta), 1e-12);
    // Lookat and eye location not looking directly at the plane, we have a little error here
    EXPECT_NEAR(-1.0, DotProduct(normal, zaxis), 1e-8);
    EXPECT_NEAR(0, DotProduct(zaxis, delta), 1e-12);
}

TEST(TransformTest, MovementInPlaneRight)
{
    using cap::Vector3D;
    using cap::Matrix3x3;
    using cap::Matrix4x4;
    using cap::HomogeneousVector3D;
    int width = 700, height = 673;
    Vector3D p1(457, 261, 0);
    Vector3D p2(457, 262, 0);
//    Vector3D eye(-249.382749654219, 225.304445635433, 266.411461553859);
    Vector3D eye(-246.913613519029, 225.304445635433, 266.411461553859);
    Vector3D lookat(53.5413164191406, -67.8972339296875, -5.17574391805289);
//    Vector3D up(-0.38759053286585, 0.378242913394051, -0.840658002578901);
    Vector3D up(-0.38874599989308, 0.379361289895661, -0.839619889769073);

    HomogeneousVector3D hclip1((p1.x-width/2.0)/(width/2.0), (p1.y-height/2.0)/(height/2.0), 0.0, 1.0);
    HomogeneousVector3D hclip2((p2.x-width/2.0)/(width/2.0), (p2.y-height/2.0)/(height/2.0), 0.0, 1.0);
    EXPECT_NEAR( 0.30571428571428572, hclip1.x, 1e-8);
    EXPECT_NEAR(-0.22436849925705796, hclip1.y, 1e-8);

    Matrix4x4 pr(1.0/300.510255242542, 0.0, 0.0, 0.0,
                 0.0, 1.0/300.510255242542, 0.0, 0.0,
                 0.0, 0.0, -2.0/(4172.74110796008-11.6763855553467), -(4172.74110796008+11.6763855553467)/(4172.74110796008-11.6763855553467),
                 0.0, 0.0, 0.0, 1.0);

    pr.InvertAffine();
    HomogeneousVector3D hdeltaeye1 = pr * hclip1;
    HomogeneousVector3D hdeltaeye2 = pr * hclip2;
    EXPECT_NEAR(-67.425034980124579, hdeltaeye1.y, 1e-8);

    Vector3D zaxis = lookat - eye;
    zaxis.Normalise();
    Vector3D xaxis = CrossProduct(up, zaxis);
    xaxis.Normalise();
    Vector3D yaxis = CrossProduct(zaxis, xaxis);
    yaxis.Normalise();

    EXPECT_NEAR(-0.83961988975102209, yaxis.z, 1e-10);

    Matrix3x3 mvrot(xaxis, yaxis, zaxis);
    mvrot.Transpose();
    Vector3D mvtr(-DotProduct(xaxis, eye), -DotProduct(yaxis, eye), -DotProduct(zaxis, eye));
    Matrix4x4 mv;
    mv.SetRotation(mvrot);
    mv.SetTranslation(mvtr);
    mv.InvertEuclidean();

    HomogeneousVector3D htlc1 = mv * hdeltaeye1;
    HomogeneousVector3D htlc2 = mv * hdeltaeye2;
    EXPECT_NEAR(-1542.0948706617144, htlc1.x, 1e-6);
    EXPECT_NEAR( 1360.8533425845542, htlc1.y, 1e-6);
    EXPECT_NEAR( 1459.4571174786531, htlc1.z, 1e-6);
    EXPECT_EQ(1.0, htlc1.w);
    Vector3D delta = Vector3D(htlc2.x, htlc2.y, htlc2.z) - Vector3D(htlc1.x, htlc1.y, htlc1.z);
    EXPECT_NEAR(-0.34716837935343392, delta.x, 1e-6);
    EXPECT_NEAR(  0.33878739392480384, delta.y, 1e-6);
    EXPECT_NEAR(-0.7498198733467234, delta.z, 1e-6);

    Vector3D normal(-0.60090985987633827, 0.58640335913024033, 0.5431744109438239);
    EXPECT_NEAR(0, DotProduct(normal, delta), 1e-12);
    // Lookat and eye location not looking directly at the plane, we have a little error here
    EXPECT_NEAR(-1.0, DotProduct(normal, zaxis), 1e-8);
    EXPECT_NEAR(0, DotProduct(zaxis, delta), 1e-12);
}

TEST(ComputeVolumeOfTetrahedronTest, Volume)
{
	using namespace cap;
	
	// computes 6 * the actual volume (to save some computation)
	//EXPECT_NEAR(ComputeVolumeOfTetrahedron(0.0f,0.0f,0.0f,
//			1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f), 1);
	
	Point3D a(0,0,0);
	Point3D b(1,0,0);
	Point3D c(0,1,0);
	Point3D d(0,0,1);
	
	EXPECT_NEAR(1, ComputeVolumeOfTetrahedron(a,b,c,d), 1e-07);
	
	d.z = -2.0f;
	EXPECT_NEAR(2, ComputeVolumeOfTetrahedron(a,b,c,d), 1e-07);
	
	d.z = -2.7f;
	EXPECT_NEAR(2.7, ComputeVolumeOfTetrahedron(a,b,c,d), 1e-07);
}

TEST(MathFunctionTest, SolveASinXPlusBCosXIsEqualToC)
{
	using namespace cap;
	
	double a = 12;
	double b = 5;
	double c = 4;
	
	double x = SolveASinXPlusBCosXIsEqualToC(a,b,c);
	double expected = 139.460 / 180.0 * M_PI;
	EXPECT_NEAR(expected, x, 0.00001);
	
	x = SolveASinXPlusBCosXIsEqualToC(10,10,10);
	expected = M_PI_2;
	EXPECT_NEAR(expected, x, 0.00001);
	
	x = SolveASinXPlusBCosXIsEqualToC(3,4,1.45099728905505);
	expected = 110.0 / 180.0 * M_PI;
	EXPECT_NEAR(expected, x, 0.00001);
}
