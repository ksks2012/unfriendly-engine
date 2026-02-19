#ifndef SATURN_RINGS_H
#define SATURN_RINGS_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

/**
 * Saturn's rings renderer using a textured disk mesh.
 * 
 * The rings are rendered as a flat disk with:
 * - Inner radius: ~1.2 Saturn radii (D Ring inner edge)
 * - Outer radius: ~2.3 Saturn radii (F Ring outer edge)
 * - Procedural radial texture with varying opacity for:
 *   - D Ring (faint, inner)
 *   - C Ring (dim)
 *   - B Ring (bright, opaque)
 *   - Cassini Division (gap, nearly transparent)
 *   - A Ring (bright)
 *   - Encke Gap (narrow gap in A Ring)
 *   - F Ring (outer, faint)
 * 
 * The ring shader is compiled once and shared across all instances
 * via static members with reference counting.
 */
class SaturnRings {
public:
    /**
     * Constructor
     * @param saturnRadius The radius of Saturn in meters
     */
    explicit SaturnRings(float saturnRadius);
    ~SaturnRings();
    
    // Prevent copying
    SaturnRings(const SaturnRings&) = delete;
    SaturnRings& operator=(const SaturnRings&) = delete;
    
    /**
     * Initialize OpenGL resources (VAO, VBO, texture, shader)
     * Must be called after OpenGL context is created
     */
    void init();
    
    /**
     * Render the rings
     * @param model Model matrix (Saturn's position and orientation)
     * @param view View matrix
     * @param projection Projection matrix
     * @param scale Rendering scale factor
     */
    void render(const glm::mat4& model, const glm::mat4& view, 
                const glm::mat4& projection, float scale) const;
    
private:
    float saturnRadius_;
    
    // Ring dimensions (in Saturn radii)
    static constexpr float INNER_RADIUS_RATIO = 1.24f;  // D Ring inner edge
    static constexpr float OUTER_RADIUS_RATIO = 2.27f;  // F Ring outer edge
    
    // Per-instance OpenGL objects
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLuint texture_ = 0;
    GLsizei indexCount_ = 0;
    
    // Shared shader program (compiled once, used by all instances)
    static GLuint sharedShaderProgram_;
    static int shaderRefCount_;
    static GLint modelLoc_;
    static GLint viewLoc_;
    static GLint projLoc_;
    static GLint textureLoc_;
    
    /**
     * Acquire the shared shader, compiling it if this is the first instance.
     */
    static void acquireShader();
    
    /**
     * Release the shared shader, deleting it if this is the last instance.
     */
    static void releaseShader();
    
    /**
     * Generate disk mesh vertices and indices
     */
    void generateDiskMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices);
    
    /**
     * Generate procedural ring texture
     * Returns a 1D texture with RGBA values representing ring structure
     */
    void generateRingTexture();
    
    /**
     * Compile and link the ring shader (called only once)
     */
    static void compileShader();
    
    /**
     * Get ring opacity at a given radial distance (in Saturn radii)
     * Models the actual ring structure including gaps
     */
    static float getRingOpacity(float radiusRatio);
    
    /**
     * Get ring color at a given radial distance
     */
    static glm::vec3 getRingColor(float radiusRatio);
};

#endif // SATURN_RINGS_H
