#include "core/octree.h"
#include <algorithm>
#include <cmath>
#include <limits>

// ============================================================
// OctreeNode implementation
// ============================================================

OctreeNode::OctreeNode(const OctreeBounds& bounds) : bounds_(bounds) {}

int OctreeNode::getOctant(const glm::vec3& position) const {
    int octant = 0;
    if (position.x >= bounds_.center.x) octant |= 4;
    if (position.y >= bounds_.center.y) octant |= 2;
    if (position.z >= bounds_.center.z) octant |= 1;
    return octant;
}

void OctreeNode::subdivide() {
    for (int i = 0; i < 8; ++i) {
        OctreeBounds childBounds = bounds_.getChildBounds(i);
        children_[i] = std::make_unique<OctreeNode>(childBounds);
        children_[i]->setDepth(depth_ + 1);
    }
    isInternal_ = true;
}

void OctreeNode::updateMassProperties(const OctreeBody& newBody) {
    // Incremental update of center of mass:
    // new_com = (old_total_mass * old_com + new_mass * new_pos) / new_total_mass
    // Use double precision to avoid overflow at solar system scales
    // (mass ~1e30 * position ~1e12 = ~1e42, which exceeds float max ~3.4e38)
    double newTotalMass = static_cast<double>(totalMass_) + static_cast<double>(newBody.mass);
    if (newTotalMass > 0.0) {
        glm::dvec3 oldWeighted = static_cast<double>(totalMass_) * glm::dvec3(centerOfMass_);
        glm::dvec3 newWeighted = static_cast<double>(newBody.mass) * glm::dvec3(newBody.position);
        glm::dvec3 newCom = (oldWeighted + newWeighted) / newTotalMass;
        centerOfMass_ = glm::vec3(newCom);
    }
    totalMass_ = static_cast<float>(newTotalMass);
    bodyCount_++;
}

void OctreeNode::insert(const OctreeBody& body) {
    // Update aggregate mass properties regardless of node state
    updateMassProperties(body);
    
    if (!hasBody_ && !isInternal_) {
        // Empty node: store the body here
        body_ = body;
        hasBody_ = true;
        return;
    }
    
    // Prevent infinite recursion if bodies are at the same position
    if (depth_ >= MAX_DEPTH) {
        // Just update mass properties (already done above), don't subdivide further
        return;
    }
    
    if (hasBody_ && !isInternal_) {
        // Leaf node with existing body: need to subdivide
        OctreeBody existingBody = body_;
        hasBody_ = false;
        
        subdivide();
        
        // Re-insert the existing body into the appropriate child
        int existingOctant = getOctant(existingBody.position);
        children_[existingOctant]->insert(existingBody);
    }
    
    // Insert new body into appropriate child
    int octant = getOctant(body.position);
    children_[octant]->insert(body);
}

glm::vec3 OctreeNode::computeAcceleration(const glm::vec3& position, float theta,
                                            float G, float softening) const {
    if (bodyCount_ == 0) {
        return glm::vec3(0.0f);
    }
    
    // Use double precision for intermediate calculations to avoid overflow
    // at solar system scales (distances ~10^12, masses ~10^30)
    glm::dvec3 delta = glm::dvec3(centerOfMass_) - glm::dvec3(position);
    double distSq = glm::dot(delta, delta);
    double dist = std::sqrt(distSq);
    
    // Skip if the query position is at the center of mass (self-interaction)
    if (dist < static_cast<double>(softening)) {
        // For leaf nodes with one body, this means we're computing force on ourselves
        if (isLeaf()) {
            return glm::vec3(0.0f);
        }
        // For internal nodes, recurse into children to handle properly
        if (isInternal_) {
            glm::vec3 acc(0.0f);
            for (const auto& child : children_) {
                if (child) {
                    acc += child->computeAcceleration(position, theta, G, softening);
                }
            }
            return acc;
        }
        return glm::vec3(0.0f);
    }
    
    // Barnes-Hut criterion: s/d < theta
    // where s = side length of the node, d = distance to center of mass
    double nodeSize = static_cast<double>(bounds_.halfSize) * 2.0;
    
    if (isLeaf() || (nodeSize / dist < static_cast<double>(theta))) {
        // Treat this node as a single body with aggregate mass
        // a = G * M / r^3 * delta (computed in double to avoid overflow)
        double distCubed = distSq * dist;
        double factor = static_cast<double>(G) * static_cast<double>(totalMass_) / distCubed;
        glm::dvec3 accD = factor * delta;
        return glm::vec3(accD);
    }
    
    // Node is too close / too large: recurse into children
    glm::vec3 acc(0.0f);
    for (const auto& child : children_) {
        if (child) {
            acc += child->computeAcceleration(position, theta, G, softening);
        }
    }
    return acc;
}

int OctreeNode::getNodeCount() const {
    int count = 1;  // Count this node
    if (isInternal_) {
        for (const auto& child : children_) {
            if (child) {
                count += child->getNodeCount();
            }
        }
    }
    return count;
}

// ============================================================
// Octree implementation
// ============================================================

Octree::Octree(float theta) : theta_(theta) {}

OctreeBounds Octree::computeBounds(const std::vector<OctreeBody>& bodies) const {
    if (bodies.empty()) {
        return OctreeBounds(glm::vec3(0.0f), 1.0f);
    }
    
    // Find the bounding box of all bodies
    glm::vec3 minPos(std::numeric_limits<float>::max());
    glm::vec3 maxPos(std::numeric_limits<float>::lowest());
    
    for (const auto& body : bodies) {
        minPos = glm::min(minPos, body.position);
        maxPos = glm::max(maxPos, body.position);
    }
    
    // Create a cube that contains all bodies
    glm::vec3 center = (minPos + maxPos) * 0.5f;
    glm::vec3 extent = maxPos - minPos;
    
    // Use the largest dimension as the cube side
    float halfSize = std::max({extent.x, extent.y, extent.z}) * 0.5f;
    
    // Add a small margin to prevent bodies from being exactly on the boundary
    halfSize *= 1.01f;
    
    // Ensure minimum size to prevent degenerate trees
    halfSize = std::max(halfSize, 1.0f);
    
    return OctreeBounds(center, halfSize);
}

void Octree::build(const std::vector<OctreeBody>& bodies) {
    if (bodies.empty()) {
        root_ = nullptr;
        return;
    }
    
    // Compute bounding box
    OctreeBounds bounds = computeBounds(bodies);
    
    // Create root and insert all bodies
    root_ = std::make_unique<OctreeNode>(bounds);
    for (const auto& body : bodies) {
        root_->insert(body);
    }
}

glm::vec3 Octree::computeAcceleration(const glm::vec3& position, float G) const {
    if (!root_) {
        return glm::vec3(0.0f);
    }
    return root_->computeAcceleration(position, theta_, G);
}

int Octree::getNodeCount() const {
    return root_ ? root_->getNodeCount() : 0;
}

int Octree::getBodyCount() const {
    return root_ ? root_->getBodyCount() : 0;
}
