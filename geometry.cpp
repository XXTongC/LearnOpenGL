#include "geometry.h"

using namespace GLframework;

std::shared_ptr<Shader> Geometry::getShader()
{
    return mShader;
}

Geometry::~Geometry()
{
	if(mVao!=0)
		glDeleteVertexArrays(1, &mVao);
	if(mPosVbo!=0)
		glDeleteBuffers(1, &mPosVbo);
	if(mUvVbo!=0)
		glDeleteBuffers(1, &mUvVbo);
	if(mColorVbo!=0)
		glDeleteBuffers(1, &mColorVbo);
	if(mNormalVbo!=0)
		glDeleteBuffers(1, &mNormalVbo);
	if (mEbo != 0)
		glDeleteBuffers(1, &mEbo);
}

void Geometry::setShader(std::shared_ptr<GLframework::Shader>shader)
{
    mShader = shader;
}

//need checck
std::shared_ptr<Geometry> Geometry::createPlane(std::shared_ptr<GLframework::Shader> shader,float width, float height)
{
	auto geometry = std::make_shared<GLframework::Geometry>(shader);
    geometry->mIndicesCount = 6;

    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    float positions[] = {
        -halfW,-halfH,0.0f,
        halfW,-halfH,0.0f,
        halfW,halfH,0.0f,
        -halfW,halfH,0.0f,
    };

    float uvs[] = {
        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
        0.0f,1.0f,
    };

    float colors[] = {
       
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 
        0.0f, 1.0f, 0.0f, 1.0f, 
    	0.0f, 0.0f, 1.0f, 1.0f,
    	0.0f, 0.0f, 1.0f, 1.0f, 
    };

    float normals[] = {
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
    };

    GLuint ebos[] = {
        0,1,2,
        2,3,0,
    };

    GLuint& ebo = geometry->mEbo,& vao =geometry->mVao,& colorVbo = geometry->mColorVbo,& uvVbo = geometry->mUvVbo,& posVbo = geometry->mPosVbo,& normalVbo = geometry->mNormalVbo;

	GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
    GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
    GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");
    GLuint normalLocation = glGetAttribLocation(shader->getProgram(), "aNormal");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(uvLocation);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glGenBuffers(1, &colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glGenBuffers(1, &normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ebos), ebos, GL_STATIC_DRAW);

    glBindVertexArray(0);

    return geometry;

}


std::shared_ptr<Geometry> Geometry::createBox(std::shared_ptr<GLframework::Shader> shader,float length, float width, float height)
{
    auto geometry = std::make_shared<GLframework::Geometry>(shader);
    geometry->mIndicesCount = 36;
	float position[]
	{
        // ǰ�� (z = height/2)
       -length / 2, -width / 2,  height / 2,  // �ײ���
        length / 2, -width / 2,  height / 2,  // �ײ���
        length / 2,  width / 2,  height / 2,  // ������
       -length / 2,  width / 2,  height / 2,  // ������

       // ���� (z = -height/2)
		-length / 2, -width / 2, -height / 2,  // �ײ���
		-length / 2,  width / 2, -height / 2,  // �ײ���
		length / 2,   width / 2, -height / 2,  // ������
		length / 2,  -width / 2, -height / 2,  // ������

        // ���� (y = width/2)
        -length / 2,  width / 2,  height / 2,  // ����
         length / 2,  width / 2,  height / 2,  // ����
         length / 2,  width / 2, -height / 2,  // ǰ��
        -length / 2,  width / 2, -height / 2,  // ǰ��

        // ���� (y = -width/2)
        -length / 2, -width / 2, -height / 2,  // ����
         length / 2, -width / 2, -height / 2,  // ����
         length / 2, -width / 2,  height / 2,  // ǰ��
        -length / 2, -width / 2,  height / 2,  // ǰ��

		// ���� (x = length/2)
        length / 2, -width / 2,  height / 2,  // �ײ���
        length / 2, -width / 2, -height / 2,  // �ײ�ǰ
        length / 2,  width / 2, -height / 2,  // ����ǰ
        length / 2,  width / 2,  height / 2,  // ������

		// ���� (x = -length/2)
		-length / 2, -width / 2, -height / 2,  // �ײ���
		-length / 2, -width / 2,  height / 2,  // �ײ�ǰ
		-length / 2,  width / 2,  height / 2,  // ����ǰ
		-length / 2,  width / 2, -height / 2,  // ������


	};
    float uvs[]
    {
        // Front face
        0.0f, 0.0f, // 0
        1.0f, 0.0f, // 1
        1.0f, 1.0f, // 2
        0.0f, 1.0f, // 3

        // Back face
        0.0f, 0.0f, // 4
        1.0f, 0.0f, // 5
        1.0f, 1.0f, // 6
        0.0f, 1.0f, // 7

        // Top face
        0.0f, 0.0f, // 8
        1.0f, 0.0f, // 9
        1.0f, 1.0f, // 10
        0.0f, 1.0f, // 11

        // Bottom face
        0.0f, 0.0f, // 12
        1.0f, 0.0f, // 13
        1.0f, 1.0f, // 14
        0.0f, 1.0f, // 15

        // Left face
        0.0f, 0.0f, // 16
        1.0f, 0.0f, // 17
        1.0f, 1.0f, // 18
        0.0f, 1.0f, // 19

        // Right face
        0.0f, 0.0f, // 20
        1.0f, 0.0f, // 21
        1.0f, 1.0f, // 22
        0.0f, 1.0f  // 23
    };
    float colors[]{
        // Front face (red)
        1.0f, 0.0f, 0.0f, 1.0f, // 0
        1.0f, 0.0f, 0.0f, 1.0f, // 1
        1.0f, 0.0f, 0.0f, 1.0f, // 2
        1.0f, 0.0f, 0.0f, 1.0f, // 3

        // Back face (green)
        0.0f, 1.0f, 0.0f, 1.0f, // 4
        0.0f, 1.0f, 0.0f, 1.0f, // 5
        0.0f, 1.0f, 0.0f, 1.0f, // 6
        0.0f, 1.0f, 0.0f, 1.0f, // 7

        // Top face (blue)
        0.0f, 0.0f, 1.0f, 1.0f, // 8
        0.0f, 0.0f, 1.0f, 1.0f, // 9
        0.0f, 0.0f, 1.0f, 1.0f, // 10
        0.0f, 0.0f, 1.0f, 1.0f, // 11

        // Bottom face (yellow)
        1.0f, 1.0f, 0.0f, 1.0f, // 12
        1.0f, 1.0f, 0.0f, 1.0f, // 13
        1.0f, 1.0f, 0.0f, 1.0f, // 14
        1.0f, 1.0f, 0.0f, 1.0f, // 15

        // Left face (cyan)
        0.0f, 1.0f, 1.0f, 1.0f, // 16
        0.0f, 1.0f, 1.0f, 1.0f, // 17
        0.0f, 1.0f, 1.0f, 1.0f, // 18
        0.0f, 1.0f, 1.0f, 1.0f, // 19

        // Right face (magenta)
        1.0f, 0.0f, 1.0f, 1.0f, // 20
        1.0f, 0.0f, 1.0f, 1.0f, // 21
        1.0f, 0.0f, 1.0f, 1.0f, // 22
        1.0f, 0.0f, 1.0f, 1.0f  // 23
    };
    //��������
    float normals[] = {
        // ǰ��
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        // ����
        0.0f,0.0f,-1.0f,
        0.0f,0.0f,-1.0f,
        0.0f,0.0f,-1.0f,
        0.0f,0.0f,-1.0f,
        // ����
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f,
        // ����
        0.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,
        // ����
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        // ����
        -1.0f,0.0f,0.0f,
        -1.0f,0.0f,0.0f,
        -1.0f,0.0f,0.0f,
        -1.0f,0.0f,0.0f,

    };

    GLuint ebos[]
	{
    	// Front face
		0, 1, 2, 2, 3, 0,
		// Back face
		4, 5, 6, 6, 7, 4,
		// Top face
		8, 9, 10, 10, 11, 8,
		// Bottom face
		12, 13, 14, 14, 15, 12,
		// Left face
		16, 17, 18, 18, 19, 16,
		// Right face
		20, 21, 22, 22, 23, 20
    };
    GLuint& vao = geometry->mVao,&posVbo = geometry->mPosVbo,&uvVbo = geometry->mUvVbo,&colorVob = geometry->mColorVbo,&ebo = geometry->mEbo,&normalvbo = geometry->mNormalVbo;
    GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
    GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
    GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");
    GLuint normalLocation = glGetAttribLocation(shader->getProgram(), "aNormal");

	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

	glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(uvLocation);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glGenBuffers(1, &colorVob);
    glBindBuffer(GL_ARRAY_BUFFER, colorVob);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glGenBuffers(1, &normalvbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ebos), ebos, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);

    glBindVertexArray(0);
    
	return geometry;
}

std::shared_ptr<Geometry> Geometry::createSphere(std::shared_ptr< GLframework::Shader> shader,float radius, int mLatitude, int mLong)
{
	auto geometry = std::make_shared<GLframework::Geometry>(shader);
    std::vector<float>vertices;
    std::vector<float>uvs;
    std::vector<float>colors;
    std::vector<float> normals;
    std::vector<int>ebos;
    //����position,uvs��colors
    for(int i = 0;i<=mLatitude;i++)
    {
	    for(int j = 0;j<=mLong;j++)
	    {
            float phi = glm::pi<float>() * static_cast<float>(i)/ static_cast<float>(mLatitude);
            float theta = 2 * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(mLong);

            float x = radius * std::sin(phi) * std::cos(theta);
            float z = radius * std::sin(phi) * std::sin(theta);
            float y = radius * cos(phi);

            float v = 1.0f - static_cast<float>(i) / static_cast<float>(mLatitude);
            float u = 1.0f - static_cast<float>(j) / static_cast<float>(mLong);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            uvs.push_back(u);
            uvs.push_back(v);

            colors.push_back((x+1)/2);
            colors.push_back((y+1)/2);
            colors.push_back((z+1)/2);
            colors.push_back(1.0f);

            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);

	    }
    }
    //����ebo
    for(int i = 0;i<mLatitude;i++)
    {
	    for(int j = 0;j<mLong;j++)
	    {
            int p1 = i * (mLong + 1) + j;
            int p2 = p1 + (mLong + 1);
            int p3 = p1 + 1;
            int p4 = p2 + 1;

            ebos.push_back(p1);
            ebos.push_back(p2);
            ebos.push_back(p3);

            ebos.push_back(p3);
            ebos.push_back(p2);
            ebos.push_back(p4);

	    }
    }

    GLuint& vao = geometry->mVao, &posVbo = geometry->mPosVbo, &uvVbo = geometry->mUvVbo, &colorVob = geometry->mColorVbo, &ebo = geometry->mEbo,&normalVbo = geometry->mNormalVbo;
    GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
    GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
    GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");
    GLuint normalLocation = glGetAttribLocation(shader->getProgram(), "aNormal");

	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(uvLocation);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glGenBuffers(1, &colorVob);
    glBindBuffer(GL_ARRAY_BUFFER, colorVob);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glGenBuffers(1, &normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebos.size() * sizeof(int), ebos.data(), GL_STATIC_DRAW);

    geometry->mIndicesCount = ebos.size();
    glBindVertexArray(0);
	return geometry;
}


Geometry::Geometry(
    std::shared_ptr<GLframework::Shader> shader,
    const std::vector<float>& positions,
    const std::vector<float>& normals,
    const std::vector<float>& uvs,
    const std::vector<unsigned int>& indices
)
{
    mShader = shader;
    mIndicesCount = indices.size();
   
    GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
    GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
    GLuint normalLocation = glGetAttribLocation(shader->getProgram(), "aNormal");


    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &mUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mUvVbo);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(uvLocation);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    glGenBuffers(1, &mNormalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mNormalVbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(float),normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);

    glBindVertexArray(0);
}


std::shared_ptr<Geometry> Geometry::createScreenPlane(std::shared_ptr<GLframework::Shader> shader)
{
    std::shared_ptr<Geometry> geometry = std::make_shared<Geometry>(shader);
    geometry->mIndicesCount = 6;

    // ����positions uv
    float positions[] = {
        -1.0f,   1.0f,
        -1.0f,  -1.0f,
         1.0f,  -1.0f,
         1.0f,   1.0f,
    };

    float uvs[] = {
        0.0f,1.0f,
    	0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
    };

    unsigned int indices[] = {
        0,1,2,
        0,2,3,
    };
    // ����vao vbo��
    GLuint& posVbo = geometry->mPosVbo, & uvVbo = geometry->mUvVbo,&ebo = geometry->mEbo,&vao = geometry->mVao;
    GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
    GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");

    glGenVertexArrays(1, &geometry->mVao);
    glBindVertexArray(vao);

	glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(uvLocation);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry->getIndicesCount() * sizeof(int), indices, GL_STATIC_DRAW);

    return geometry;
}

Geometry::Geometry(
    std::shared_ptr<GLframework::Shader> shader,
    const std::vector<float>& positions,
    const std::vector<float>& normals,
    const std::vector<float>& uvs,
    const std::vector<float>& colors,
    const std::vector<unsigned int>& indices
)
{
    mShader = shader;
    mIndicesCount = indices.size();

    GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
    GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
    GLuint normalLocation = glGetAttribLocation(shader->getProgram(), "aNormal");
    GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mColorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &mUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mUvVbo);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(uvLocation);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    glGenBuffers(1, &mNormalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mNormalVbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);

    glBindVertexArray(0);
}