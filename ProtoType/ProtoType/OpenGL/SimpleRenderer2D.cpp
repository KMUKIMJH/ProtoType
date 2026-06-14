#include "SimpleRenderer2D.h"
#include "../Engine/Matrix.h"
#include "../Engine/Vec2.h"
#include "DrawUtils2D.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <GL/glew.h>

namespace GAME200
{
    namespace
    {
        std::string load_text_file(const std::filesystem::path& file)
        {
            std::ifstream ifs(file, std::ios::in | std::ios::binary);
            if (!ifs)
            {
                return {};
            }
            std::string s;
            ifs.seekg(0, std::ios::end);
            s.resize(static_cast<size_t>(ifs.tellg()));
            ifs.seekg(0, std::ios::beg);
            ifs.read(s.data(), static_cast<std::streamsize>(s.size()));
            return s;
        }

        GLuint compile(GLenum type, const char* src)
        {
            GLuint sh = glCreateShader(type);
            glShaderSource(sh, 1, &src, nullptr);
            glCompileShader(sh);
            GLint ok = GL_FALSE;
            glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
            if (!ok)
            {
                GLint len = 0; 
                glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
                std::string log; 
                log.resize(static_cast<size_t>(len) + 1);
                glGetShaderInfoLog(sh, len, nullptr, log.data());
                glDeleteShader(sh);
                throw std::runtime_error(log);
            }
            return sh;
        }

        GLuint link_program(GLuint vs, GLuint fs)
        {
            GLuint prog = glCreateProgram();
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
            glLinkProgram(prog);
            glDeleteShader(vs);
            glDeleteShader(fs);
            GLint ok = GL_FALSE;
            glGetProgramiv(prog, GL_LINK_STATUS, &ok);
            if (!ok)
            {
                GLint len = 0; 
                glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
                std::string log; 
                log.resize(static_cast<size_t>(len) + 1);
                glGetProgramInfoLog(prog, len, nullptr, log.data());
                glDeleteProgram(prog);
                throw std::runtime_error(log);
            }
            return prog;
        }
    }

    SimpleRenderer2D::SimpleRenderer2D(SimpleRenderer2D&& other) noexcept
        : fixedVBO(other.fixedVBO)
        , instanceVBO(other.instanceVBO)
        , indexBuffer(other.indexBuffer)
        , quadVAO(other.quadVAO)
        , quadShader(other.quadShader)
        , cameraUBO(other.cameraUBO)
        , sdfVAO(other.sdfVAO)
        , sdfVBO(other.sdfVBO)
        , sdfShader(other.sdfShader)
        , sdf_u_model_matrix(other.sdf_u_model_matrix)
        , sdf_u_shapeSize(other.sdf_u_shapeSize)
        , sdf_u_fillColor(other.sdf_u_fillColor)
        , sdf_u_lineColor(other.sdf_u_lineColor)
        , sdf_u_lineWidth(other.sdf_u_lineWidth)
        , sdf_u_shapeType(other.sdf_u_shapeType)
        , instances(std::move(other.instances))
        , textureSlots(std::move(other.textureSlots))
        , activeTextureCount(other.activeTextureCount)
        , maxInstances(other.maxInstances)
        , drawCallCount(other.drawCallCount)
        , flushCount(other.flushCount)
        , submittedInstanceTotal(other.submittedInstanceTotal)
        , viewProjectionMatrix(other.viewProjectionMatrix)
    {
        other.fixedVBO = 0;
        other.instanceVBO = 0;
        other.indexBuffer = 0;
        other.quadVAO = 0;
        other.quadShader = 0;
        other.cameraUBO = 0;
        other.sdfVAO = 0;
        other.sdfVBO = 0;
        other.sdfShader = 0;
    }

    SimpleRenderer2D& SimpleRenderer2D::operator=(SimpleRenderer2D&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        Shutdown();
        fixedVBO = other.fixedVBO; other.fixedVBO = 0;
        instanceVBO = other.instanceVBO; other.instanceVBO = 0;
        indexBuffer = other.indexBuffer; other.indexBuffer = 0;
        quadVAO = other.quadVAO; other.quadVAO = 0;
        quadShader = other.quadShader; other.quadShader = 0;
        cameraUBO = other.cameraUBO; other.cameraUBO = 0;
        sdfVAO = other.sdfVAO; other.sdfVAO = 0;
        sdfVBO = other.sdfVBO; other.sdfVBO = 0;
        sdfShader = other.sdfShader; other.sdfShader = 0;
        sdf_u_model_matrix = other.sdf_u_model_matrix;
        sdf_u_shapeSize = other.sdf_u_shapeSize;
        sdf_u_fillColor = other.sdf_u_fillColor;
        sdf_u_lineColor = other.sdf_u_lineColor;
        sdf_u_lineWidth = other.sdf_u_lineWidth;
        sdf_u_shapeType = other.sdf_u_shapeType;
        instances = std::move(other.instances);
        textureSlots = std::move(other.textureSlots);
        activeTextureCount = other.activeTextureCount;
        maxInstances = other.maxInstances;
        drawCallCount = other.drawCallCount;
        flushCount = other.flushCount;
        submittedInstanceTotal = other.submittedInstanceTotal;
        viewProjectionMatrix = other.viewProjectionMatrix;
        return *this;
    }

    SimpleRenderer2D::~SimpleRenderer2D()
    {
        Shutdown();
    }

    void SimpleRenderer2D::Init()
    {
        GLint max_tex_units = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_tex_units);
        const size_t shader_slots = 8;
        const size_t tex_slot_count = std::min<size_t>(shader_slots, static_cast<size_t>(std::max(0, max_tex_units)));
        textureSlots.resize(tex_slot_count);

        std::string quad_vs_src = load_text_file("Assets/shaders/quad.vert");
        std::string quad_fs_src = load_text_file("Assets/shaders/quad.frag");
        GLuint quad_vs = compile(GL_VERTEX_SHADER, quad_vs_src.c_str());
        GLuint quad_fs = compile(GL_FRAGMENT_SHADER, quad_fs_src.c_str());
        quadShader = link_program(quad_vs, quad_fs);

        glGenBuffers(1, &cameraUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        if (GLuint camIndex = glGetUniformBlockIndex(quadShader, "Camera"); camIndex != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(quadShader, camIndex, 0);
        }

        constexpr float quad_vertices[] =
        {
            -0.5f, -0.5f, 0.0f, 0.0f,
             0.5f, -0.5f, 1.0f, 0.0f,
             0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 1.0f
        };
        constexpr unsigned char indices[] = { 0, 1, 2, 0, 2, 3 };

        glGenBuffers(1, &fixedVBO);
        glBindBuffer(GL_ARRAY_BUFFER, fixedVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(QuadInstance) * maxInstances), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fixedVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(QuadInstance), reinterpret_cast<void*>(offsetof(QuadInstance, transformRow0)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(QuadInstance), reinterpret_cast<void*>(offsetof(QuadInstance, transformRow1)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(QuadInstance), reinterpret_cast<void*>(offsetof(QuadInstance, tint)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(QuadInstance), reinterpret_cast<void*>(offsetof(QuadInstance, texScale)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(QuadInstance), reinterpret_cast<void*>(offsetof(QuadInstance, texOffset)));
        glEnableVertexAttribArray(7);
        glVertexAttribIPointer(7, 1, GL_INT, sizeof(QuadInstance), reinterpret_cast<void*>(offsetof(QuadInstance, textureIndex)));
        for (GLuint loc = 2; loc <= 7; ++loc)
        {
            glVertexAttribDivisor(loc, 1);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBindVertexArray(0);

        glUseProgram(quadShader);
        std::vector<int> sampler_indices(textureSlots.size());
        std::iota(sampler_indices.begin(), sampler_indices.end(), 0);
        if (GLint texArrayLoc = glGetUniformLocation(quadShader, "uTextures"); texArrayLoc >= 0)
        {
            glUniform1iv(texArrayLoc, static_cast<GLsizei>(sampler_indices.size()), sampler_indices.data());
        }
        glUseProgram(0);

        std::string sdf_vs_src = load_text_file("Assets/shaders/sdf.vert");
        std::string sdf_fs_src = load_text_file("Assets/shaders/sdf.frag");
        GLuint sdf_vs = compile(GL_VERTEX_SHADER, sdf_vs_src.c_str());
        GLuint sdf_fs = compile(GL_FRAGMENT_SHADER, sdf_fs_src.c_str());
        sdfShader = link_program(sdf_vs, sdf_fs);
        if (GLuint sdfCamIndex = glGetUniformBlockIndex(sdfShader, "Camera"); sdfCamIndex != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(sdfShader, sdfCamIndex, 0);
        }
        glUseProgram(sdfShader);
        sdf_u_model_matrix = glGetUniformLocation(sdfShader, "u_model_matrix");
        sdf_u_shapeSize = glGetUniformLocation(sdfShader, "u_shapeSize");
        sdf_u_size = glGetUniformLocation(sdfShader, "u_size");
        sdf_u_fillColor = glGetUniformLocation(sdfShader, "u_fillColor");
        sdf_u_lineColor = glGetUniformLocation(sdfShader, "u_lineColor");
        sdf_u_lineWidth = glGetUniformLocation(sdfShader, "u_lineWidth");
        sdf_u_shapeType = glGetUniformLocation(sdfShader, "u_shapeType");
        sdf_u_worldPx = glGetUniformLocation(sdfShader, "u_worldPx");
        glUseProgram(0);

        struct vec2f
        {
            float x; 
            float y;
        };
        const vec2f sdf_vertices[] = { { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } };
        glGenVertexArrays(1, &sdfVAO);
        glBindVertexArray(sdfVAO);
        glGenBuffers(1, &sdfVBO);
        glBindBuffer(GL_ARRAY_BUFFER, sdfVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sdf_vertices), sdf_vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2f), reinterpret_cast<void*>(0));
        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        instances.reserve(maxInstances);
    }

    void SimpleRenderer2D::Shutdown()
    {
        if (quadVAO)
        {
            glDeleteVertexArrays(1, &quadVAO);
        }
        if (fixedVBO)
        {
            glDeleteBuffers(1, &fixedVBO);
        }
        if (instanceVBO)
        {
            glDeleteBuffers(1, &instanceVBO);
        }
        if (indexBuffer)
        {
            glDeleteBuffers(1, &indexBuffer);
        }
        if (quadShader)
        {
            glDeleteProgram(quadShader);
        }
        if (cameraUBO)
        {
            glDeleteBuffers(1, &cameraUBO);
        }
        if (sdfVAO)
        {
            glDeleteVertexArrays(1, &sdfVAO);
        }
        if (sdfVBO)
        {
            glDeleteBuffers(1, &sdfVBO);
        }
        if (sdfShader)
        {
            glDeleteProgram(sdfShader);
        }
        quadVAO = 0;
        fixedVBO = 0;
        instanceVBO = 0;
        indexBuffer = 0;
        quadShader = 0;
        cameraUBO = 0;
        sdfVAO = 0;
        sdfVBO = 0;
        sdfShader = 0;
    }

    void SimpleRenderer2D::BeginScene(const Math::TransformationMatrix& view_projection)
    {
        viewProjectionMatrix = view_projection;
        const auto& m = viewProjectionMatrix;
        std::array<float, 12> data =
        {
            static_cast<float>(m[0][0]), static_cast<float>(m[1][0]), static_cast<float>(m[2][0]), 0.f,
            static_cast<float>(m[0][1]), static_cast<float>(m[1][1]), static_cast<float>(m[2][1]), 0.f,
            static_cast<float>(m[0][2]), static_cast<float>(m[1][2]), static_cast<float>(m[2][2]), 0.f
        };
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 12, data.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        flushCount = 0;
        submittedInstanceTotal = 0;
        drawCallCount = 0;
        startBatch();
    }

    void SimpleRenderer2D::EndScene()
    {
        flush();
    }

    void SimpleRenderer2D::startBatch()
    {
        instances.clear();
        activeTextureCount = 0;
    }

    void SimpleRenderer2D::flush()
    {
        if (instances.empty())
        {
            return;
        }
        submittedInstanceTotal += static_cast<unsigned>(instances.size());
        ++flushCount;

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(sizeof(QuadInstance) * instances.size()), instances.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        for (size_t i = 0; i < activeTextureCount; ++i)
        {
            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i));
            glBindTexture(GL_TEXTURE_2D, textureSlots[i]);
        }

        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        glUseProgram(quadShader);
        glBindVertexArray(quadVAO);
        ++drawCallCount;
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, static_cast<GLsizei>(instances.size()));
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        startBatch();
    }

    void SimpleRenderer2D::DrawQuad(const Math::TransformationMatrix& transform, unsigned int texture, Math::vec2 bl, Math::vec2 tr, GAME200::RGBA tint)
    {
        if (instances.size() >= maxInstances)
        {
            flush();
        }

        int tex_index = -1;
        for (size_t i = 0; i < activeTextureCount; ++i)
        {
            if (textureSlots[i] == texture)
            {
                tex_index = static_cast<int>(i);
                break;
            }
        }
        if (tex_index < 0)
        {
            if (activeTextureCount >= textureSlots.size())
            {
                flush();
            }
            tex_index = static_cast<int>(activeTextureCount);
            if (tex_index >= static_cast<int>(textureSlots.size()))
            {
                return;
            }
            textureSlots[activeTextureCount] = texture;
            ++activeTextureCount;
        }

        QuadInstance inst{};
        inst.textureIndex = tex_index;
        inst.texScale[0] = static_cast<float>(tr.x - bl.x);
        inst.texScale[1] = static_cast<float>(tr.y - bl.y);
        inst.texOffset[0] = static_cast<float>(bl.x);
        inst.texOffset[1] = static_cast<float>(bl.y);
        inst.transformRow0[0] = static_cast<float>(transform[0][0]);
        inst.transformRow0[1] = static_cast<float>(transform[0][1]);
        inst.transformRow0[2] = static_cast<float>(transform[0][2]);
        inst.transformRow1[0] = static_cast<float>(transform[1][0]);
        inst.transformRow1[1] = static_cast<float>(transform[1][1]);
        inst.transformRow1[2] = static_cast<float>(transform[1][2]);
        inst.tint[0] = static_cast<unsigned char>((tint >> 24) & 0xFF);
        inst.tint[1] = static_cast<unsigned char>((tint >> 16) & 0xFF);
        inst.tint[2] = static_cast<unsigned char>((tint >> 8) & 0xFF);
        inst.tint[3] = static_cast<unsigned char>(tint & 0xFF);
        instances.push_back(inst);
    }

    void SimpleRenderer2D::DrawCircle(const Math::TransformationMatrix& t, GAME200::RGBA fill, GAME200::RGBA line, double w)
    {
        DrawSDF(t, fill, line, w, SDFShape::Circle);
    }

    void SimpleRenderer2D::DrawRectangle(const Math::TransformationMatrix& t, GAME200::RGBA fill, GAME200::RGBA line, double w)
    {
        DrawSDF(t, fill, line, w, SDFShape::Rectangle);
    }

    void SimpleRenderer2D::DrawLine(const Math::TransformationMatrix& t, Math::vec2 a, Math::vec2 b, GAME200::RGBA color, double w)
    {
        const auto line_t = DrawUtils2D::CalculateLineTransform(t, a, b, w);
        DrawSDF(line_t, GAME200::CLEAR, color, w, SDFShape::Rectangle);
    }

    void SimpleRenderer2D::DrawSDF(const Math::TransformationMatrix& transform, GAME200::RGBA fill_color, GAME200::RGBA line_color, double line_width, SDFShape shape)
    {
        flush(); // Ensure batched geometry is drawn before immediate geometry to preserve order

        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);

        glUseProgram(sdfShader);

        const double sx = std::sqrt(transform[0][0] * transform[0][0] + transform[1][0] * transform[1][0]);
        const double sy = std::sqrt(transform[0][1] * transform[0][1] + transform[1][1] * transform[1][1]);
        const bool has_outline = (line_width > 0.0) && ((line_color & 0xFFu) != 0);

        GLint vp[4] = { 0, 0, 1, 1 };
        glGetIntegerv(GL_VIEWPORT, vp);
        const double ndcPerPixelX = 2.0 / std::max(1, vp[2]);
        const double ndcPerPixelY = 2.0 / std::max(1, vp[3]);
        const auto& m = viewProjectionMatrix;
        const double worldToNdcX = std::sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);
        const double worldToNdcY = std::sqrt(m[0][1] * m[0][1] + m[1][1] * m[1][1]);
        double pxWorldX = 0.0;
        if (worldToNdcX > 1e-12)
        {
            pxWorldX = ndcPerPixelX / worldToNdcX;
        }
        double pxWorldY = 0.0;
        if (worldToNdcY > 1e-12)
        {
            pxWorldY = ndcPerPixelY / worldToNdcY;
        }
        const double pxWorld = std::max(pxWorldX, pxWorldY);

        double pad = 0.0;
        if (shape == SDFShape::Circle)
        {
            if (has_outline)
            {
                pad = line_width + 2.0 * pxWorld;
            }
            else
            {
                pad = 2.0 * pxWorld;
            }
        }
        else
        {
            if (has_outline)
            {
                pad = (0.5 * line_width) + pxWorld;
            }
            else
            {
                pad = 0.0;
            }
        }

        const double qx = sx + 2.0 * pad;
        const double qy = sy + 2.0 * pad;

        const double rot = std::atan2(transform[1][0], transform[0][0]);
        const double tx = transform[0][2];
        const double ty = transform[1][2];
        const float c = static_cast<float>(std::cos(rot));
        const float s = static_cast<float>(std::sin(rot));
        std::array<float, 9> model =
        {
            c * static_cast<float>(qx),  s * static_cast<float>(qx), 0.0f,
            -s * static_cast<float>(qy), c * static_cast<float>(qy), 0.0f,
            static_cast<float>(tx),      static_cast<float>(ty),     1.0f
        };
        glUniformMatrix3fv(sdf_u_model_matrix, 1, GL_FALSE, model.data());
        glUniform2f(sdf_u_shapeSize, static_cast<float>(sx), static_cast<float>(sy));
        if (sdf_u_size >= 0)
        {
            glUniform2f(sdf_u_size, static_cast<float>(qx), static_cast<float>(qy));
        }

        if (sdf_u_lineWidth >= 0)
        {
            glUniform1f(sdf_u_lineWidth, static_cast<float>(line_width));
        }

        auto unpack = [](GAME200::RGBA cval)
        {
            return std::array<float, 4>
            {
                static_cast<float>((cval >> 24) & 0xFF) / 255.0f,
                static_cast<float>((cval >> 16) & 0xFF) / 255.0f,
                static_cast<float>((cval >> 8) & 0xFF) / 255.0f,
                static_cast<float>(cval & 0xFF) / 255.0f
            };
        };
        auto fillF = unpack(fill_color);
        glUniform4fv(sdf_u_fillColor, 1, fillF.data());
        auto lineF = unpack(line_color);
        glUniform4fv(sdf_u_lineColor, 1, lineF.data());
        glUniform1i(sdf_u_shapeType, static_cast<int>(shape));

        const float worldPx = static_cast<float>(pxWorld);
        if (sdf_u_worldPx >= 0)
        {
            glUniform1f(sdf_u_worldPx, worldPx);
        }

        glBindVertexArray(sdfVAO);
        ++drawCallCount;
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}
