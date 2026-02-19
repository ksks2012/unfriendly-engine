#ifndef OCTREE_H
#define OCTREE_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <array>
#include <string>

/**
 * Barnes-Hut Octree for efficient N-body gravitational force calculation.
 *
 * Instead of O(n^2) direct summation, the Barnes-Hut algorithm builds a
 * spatial octree and approximates distant groups of bodies as single
 * point masses, reducing complexity to O(n log n).
 *
 * The opening angle parameter (theta) controls accuracy:
 *   theta = 0.0 : exact direct summation (no approximation)
 *   theta = 0.5 : typical good balance of speed and accuracy
 *   theta = 1.0 : aggressive approximation, faster but less accurate
 */

/**
 * Represents a body (particle) inserted into the octree.
 * Uses lightweight value type to avoid coupling with Body class.
 */
struct OctreeBody {
    glm::dvec3 position;
    double mass;
    std::string name;  // For debugging / identification
    
    OctreeBody() : position(0.0), mass(0.0), name("") {}
    OctreeBody(const glm::dvec3& pos, double m, const std::string& n = "")
        : position(pos), mass(m), name(n) {}
};

/**
 * Axis-aligned bounding box for octree nodes.
 */
struct OctreeBounds {
    glm::dvec3 center;
    double halfSize;  // Half the side length of the cube
    
    OctreeBounds() : center(0.0), halfSize(0.0) {}
    OctreeBounds(const glm::dvec3& c, double h) : center(c), halfSize(h) {}
    
    /**
     * Check if a point is inside this bounding box.
     */
    bool contains(const glm::dvec3& point) const {
        return (point.x >= center.x - halfSize && point.x <= center.x + halfSize &&
                point.y >= center.y - halfSize && point.y <= center.y + halfSize &&
                point.z >= center.z - halfSize && point.z <= center.z + halfSize);
    }
    
    /**
     * Get the child octant bounds for a given octant index (0-7).
     * Octant layout:
     *   0: -x, -y, -z    4: +x, -y, -z
     *   1: -x, -y, +z    5: +x, -y, +z
     *   2: -x, +y, -z    6: +x, +y, -z
     *   3: -x, +y, +z    7: +x, +y, +z
     */
    OctreeBounds getChildBounds(int octant) const {
        double quarter = halfSize * 0.5;
        glm::dvec3 childCenter = center;
        childCenter.x += (octant & 4) ? quarter : -quarter;
        childCenter.y += (octant & 2) ? quarter : -quarter;
        childCenter.z += (octant & 1) ? quarter : -quarter;
        return OctreeBounds(childCenter, quarter);
    }
};

/**
 * A node in the Barnes-Hut octree.
 * Each node represents a cubic region of space and can be:
 *   - Empty: no bodies
 *   - Leaf: exactly one body
 *   - Internal: has children (subdivided)
 */
class OctreeNode {
public:
    OctreeNode(const OctreeBounds& bounds);
    ~OctreeNode() = default;
    
    /**
     * Insert a body into this node.
     * If the node already contains a body, it will subdivide.
     */
    void insert(const OctreeBody& body);
    
    /**
     * Calculate gravitational acceleration on a body at the given position
     * using the Barnes-Hut approximation.
     *
     * @param position Position of the body to calculate force on
     * @param theta Opening angle parameter (0 = exact, 0.5 = typical)
     * @param G Gravitational constant
     * @param softening Softening length to prevent singularities
     * @return Gravitational acceleration vector
     */
    glm::dvec3 computeAcceleration(const glm::dvec3& position, double theta, 
                                   double G, double softening = 1e-6) const;
    
    // Accessors for testing
    bool isEmpty() const { return !hasBody_ && !isInternal_; }
    bool isLeaf() const { return hasBody_ && !isInternal_; }
    bool isInternal() const { return isInternal_; }
    double getTotalMass() const { return totalMass_; }
    glm::dvec3 getCenterOfMass() const { return centerOfMass_; }
    const OctreeBounds& getBounds() const { return bounds_; }
    int getBodyCount() const { return bodyCount_; }
    
    /**
     * Get total number of nodes in the tree (for diagnostics).
     */
    int getNodeCount() const;
    
private:
    OctreeBounds bounds_;
    
    // Aggregate mass properties (used for Barnes-Hut approximation)
    double totalMass_ = 0.0;
    glm::dvec3 centerOfMass_ = glm::dvec3(0.0);
    int bodyCount_ = 0;
    
    // Node state
    bool hasBody_ = false;         // True if this leaf contains exactly one body
    bool isInternal_ = false;      // True if this node has been subdivided
    OctreeBody body_;              // The body stored in this leaf (only valid if hasBody_)
    
    // Children (8 octants), only allocated when subdivided
    std::array<std::unique_ptr<OctreeNode>, 8> children_;
    
    /**
     * Determine which octant a position falls into.
     */
    int getOctant(const glm::dvec3& position) const;
    
    /**
     * Subdivide this node into 8 children.
     */
    void subdivide();
    
    /**
     * Update the aggregate mass and center of mass.
     */
    void updateMassProperties(const OctreeBody& newBody);
    
    // Maximum recursion depth to prevent infinite subdivision
    // when two bodies are at the exact same position
    static constexpr int MAX_DEPTH = 40;
    int depth_ = 0;
    
    void setDepth(int d) { depth_ = d; }
};

/**
 * Barnes-Hut Octree manager.
 * Handles tree construction, force calculation, and provides
 * a clean interface for the simulation.
 */
class Octree {
public:
    /**
     * Constructor
     * @param theta Opening angle parameter (default 0.5)
     */
    explicit Octree(float theta = 0.5f);
    ~Octree() = default;
    
    /**
     * Build the octree from a collection of bodies.
     * This rebuilds the tree from scratch each time.
     *
     * @param bodies Vector of bodies to insert
     */
    void build(const std::vector<OctreeBody>& bodies);
    
    /**
     * Calculate gravitational acceleration on a body at the given position.
     *
     * @param position Position of the body
     * @param G Gravitational constant
     * @return Gravitational acceleration vector
     */
    glm::dvec3 computeAcceleration(const glm::dvec3& position, double G) const;
    
    /**
     * Set the opening angle parameter.
     */
    void setTheta(float theta) { theta_ = theta; }
    float getTheta() const { return theta_; }
    
    /**
     * Get diagnostics about the current tree.
     */
    int getNodeCount() const;
    int getBodyCount() const;
    bool isBuilt() const { return root_ != nullptr; }
    
private:
    float theta_;
    std::unique_ptr<OctreeNode> root_;
    
    /**
     * Calculate bounding box that contains all bodies.
     */
    OctreeBounds computeBounds(const std::vector<OctreeBody>& bodies) const;
};

#endif // OCTREE_H