#include "core/octree.h"
#include <algorithm>
#include <cmath>
#include <limits>

// ============================================================
// OctreeNode implementation
// ============================================================

OctreeNode::OctreeNode(const OctreeBounds& bounds) : bounds_(bounds) {}

int OctreeNode::getOctant(const glm::dvec3& position) const {
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
    // All values are now double precision natively, no conversion needed.
    double newTotalMass = totalMass_ + newBody.mass;
    if (newTotalMass > 0.0) {
        glm::dvec3 oldWeighted = totalMass_ * centerOfMass_;
        glm::dvec3 newWeighted = newBody.mass * newBody.position;
        centerOfMass_ = (oldWeighted + newWeighted) / newTotalMass;
    }
    totalMass_ = newTotalMass;
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

glm::dvec3 OctreeNode::computeAcceleration(const glm::dvec3& position, double theta,
                                            double G, double softening) const {
    if (bodyCount_ == 0) {
        return glm::dvec3(0.0);
    }
    
    glm::dvec3 delta = centerOfMass_ - position;
    double distSq = glm::dot(delta, delta);
    double dist = std::sqrt(distSq);
    
    // Skip if the query position is at the center of mass (self-interaction)
    if (dist < softening) {
        // For leaf nodes with one body, this means we're computing force on ourselves
        if (isLeaf()) {
            return glm::dvec3(0.0);
        }
        // For internal nodes, recurse into children to handle properly
        if (isInternal_) {
            glm::dvec3 acc(0.0);
            for (const auto& child : children_) {
                if (child) {
                    acc += child->computeAcceleration(position, theta, G, softening);
                }
            }
            return acc;
        }
        return glm::dvec3(0.0);
    }
    
    // Barnes-Hut criterion: s/d < theta
    // where s = side length of the node, d = distance to center of mass
    double nodeSize = bounds_.halfSize * 2.0;
    
    if (isLeaf() || (nodeSize / dist < theta)) {
        // Treat this node as a single body with aggregate mass
        // a = G * M / r^3 * delta
        double distCubed = distSq * dist;
        double factor = G * totalMass_ / distCubed;
        return factor * delta;
    }
    
    // Node is too close / too large: recurse into children
    glm::dvec3 acc(0.0);
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
        return OctreeBounds(glm::dvec3(0.0), 1.0);
    }
    
    // Find the bounding box of all bodies
    glm::dvec3 minPos(std::numeric_limits<double>::max());
    glm::dvec3 maxPos(std::numeric_limits<double>::lowest());
    
    for (const auto& body : bodies) {
        minPos = glm::min(minPos, body.position);
        maxPos = glm::max(maxPos, body.position);
    }
    
    // Create a cube that contains all bodies
    glm::dvec3 center = (minPos + maxPos) * 0.5;
    glm::dvec3 extent = maxPos - minPos;
    
    // Use the largest dimension as the cube side
    double halfSize = std::max({extent.x, extent.y, extent.z}) * 0.5;
    
    // Add a small margin to prevent bodies from being exactly on the boundary
    halfSize *= 1.01;
    
    // Ensure minimum size to prevent degenerate trees
    halfSize = std::max(halfSize, 1.0);
    
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

glm::dvec3 Octree::computeAcceleration(const glm::dvec3& position, double G) const {
    if (!root_) {
        return glm::dvec3(0.0);
    }
    return root_->computeAcceleration(position, theta_, G);
}

int Octree::getNodeCount() const {
    return root_ ? root_->getNodeCount() : 0;
}

int Octree::getBodyCount() const {
    return root_ ? root_->getBodyCount() : 0;
}
