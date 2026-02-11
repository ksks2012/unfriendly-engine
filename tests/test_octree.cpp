#include "core/octree.h"

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <cmath>
#include <random>
#include <chrono>
#include <iostream>

// Gravitational constant used in the project
static constexpr float G = 6.674e-11f;

// ============================================================
// Helper: Direct N-body force calculation for comparison
// ============================================================

/**
 * Compute gravitational acceleration on a body using direct summation (O(n^2)).
 * This serves as the ground truth for validating Barnes-Hut results.
 * Uses double precision internally to handle solar system scales.
 */
glm::vec3 directAcceleration(const glm::vec3& position, 
                              const std::vector<OctreeBody>& bodies,
                              float gravitationalConstant) {
    glm::dvec3 acc(0.0);
    double G_d = static_cast<double>(gravitationalConstant);
    glm::dvec3 pos_d = glm::dvec3(position);
    
    for (const auto& body : bodies) {
        glm::dvec3 delta = glm::dvec3(body.position) - pos_d;
        double distSq = glm::dot(delta, delta);
        double dist = std::sqrt(distSq);
        if (dist > 1e-6) {
            double distCubed = distSq * dist;
            acc += (G_d * static_cast<double>(body.mass) / distCubed) * delta;
        }
    }
    return glm::vec3(acc);
}

// ============================================================
// OctreeNode Unit Tests
// ============================================================

class OctreeNodeTest : public ::testing::Test {
protected:
    OctreeBounds defaultBounds = OctreeBounds(glm::vec3(0.0f), 100.0f);
};

TEST_F(OctreeNodeTest, EmptyNode) {
    OctreeNode node(defaultBounds);
    EXPECT_TRUE(node.isEmpty());
    EXPECT_FALSE(node.isLeaf());
    EXPECT_FALSE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 0);
    EXPECT_FLOAT_EQ(node.getTotalMass(), 0.0f);
}

TEST_F(OctreeNodeTest, InsertSingleBody) {
    OctreeNode node(defaultBounds);
    OctreeBody body(glm::vec3(10.0f, 20.0f, 30.0f), 100.0f, "test");
    
    node.insert(body);
    
    EXPECT_FALSE(node.isEmpty());
    EXPECT_TRUE(node.isLeaf());
    EXPECT_FALSE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 1);
    EXPECT_FLOAT_EQ(node.getTotalMass(), 100.0f);
    EXPECT_EQ(node.getCenterOfMass(), body.position);
}

TEST_F(OctreeNodeTest, InsertTwoBodies_Subdivides) {
    OctreeNode node(defaultBounds);
    OctreeBody body1(glm::vec3(50.0f, 50.0f, 50.0f), 100.0f, "body1");
    OctreeBody body2(glm::vec3(-50.0f, -50.0f, -50.0f), 200.0f, "body2");
    
    node.insert(body1);
    node.insert(body2);
    
    EXPECT_TRUE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 2);
    EXPECT_FLOAT_EQ(node.getTotalMass(), 300.0f);
    
    // Center of mass should be weighted average
    glm::vec3 expectedCom = (100.0f * body1.position + 200.0f * body2.position) / 300.0f;
    EXPECT_NEAR(node.getCenterOfMass().x, expectedCom.x, 1e-3f);
    EXPECT_NEAR(node.getCenterOfMass().y, expectedCom.y, 1e-3f);
    EXPECT_NEAR(node.getCenterOfMass().z, expectedCom.z, 1e-3f);
}

TEST_F(OctreeNodeTest, InsertMultipleBodies) {
    OctreeNode node(defaultBounds);
    
    node.insert(OctreeBody(glm::vec3(50.0f, 50.0f, 50.0f), 100.0f));
    node.insert(OctreeBody(glm::vec3(-50.0f, 50.0f, 50.0f), 100.0f));
    node.insert(OctreeBody(glm::vec3(50.0f, -50.0f, 50.0f), 100.0f));
    node.insert(OctreeBody(glm::vec3(-50.0f, -50.0f, -50.0f), 100.0f));
    
    EXPECT_TRUE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 4);
    EXPECT_FLOAT_EQ(node.getTotalMass(), 400.0f);
}

TEST_F(OctreeNodeTest, BodiesAtSamePosition_NoInfiniteRecursion) {
    OctreeNode node(defaultBounds);
    
    // Insert two bodies at the exact same position
    // Should not cause infinite recursion due to MAX_DEPTH limit
    node.insert(OctreeBody(glm::vec3(10.0f), 100.0f, "a"));
    node.insert(OctreeBody(glm::vec3(10.0f), 200.0f, "b"));
    
    EXPECT_EQ(node.getBodyCount(), 2);
    EXPECT_FLOAT_EQ(node.getTotalMass(), 300.0f);
}

TEST_F(OctreeNodeTest, NodeCount) {
    OctreeNode node(defaultBounds);
    
    // Empty node: just 1 (root)
    EXPECT_EQ(node.getNodeCount(), 1);
    
    // One body: still 1 (leaf)
    node.insert(OctreeBody(glm::vec3(50.0f), 100.0f));
    EXPECT_EQ(node.getNodeCount(), 1);
    
    // Two bodies: root + 8 children = 9
    node.insert(OctreeBody(glm::vec3(-50.0f), 100.0f));
    EXPECT_EQ(node.getNodeCount(), 9);
}

// ============================================================
// OctreeBounds Tests
// ============================================================

TEST(OctreeBoundsTest, Contains) {
    OctreeBounds bounds(glm::vec3(0.0f), 100.0f);
    
    EXPECT_TRUE(bounds.contains(glm::vec3(0.0f)));
    EXPECT_TRUE(bounds.contains(glm::vec3(50.0f, 50.0f, 50.0f)));
    EXPECT_TRUE(bounds.contains(glm::vec3(-99.0f, -99.0f, -99.0f)));
    EXPECT_TRUE(bounds.contains(glm::vec3(100.0f, 100.0f, 100.0f)));  // On boundary
    
    EXPECT_FALSE(bounds.contains(glm::vec3(101.0f, 0.0f, 0.0f)));
    EXPECT_FALSE(bounds.contains(glm::vec3(0.0f, -101.0f, 0.0f)));
}

TEST(OctreeBoundsTest, ChildBounds) {
    OctreeBounds parent(glm::vec3(0.0f), 100.0f);
    
    // Child octant 7 (+x, +y, +z) should have center at (50, 50, 50) with halfSize 50
    OctreeBounds child7 = parent.getChildBounds(7);
    EXPECT_FLOAT_EQ(child7.center.x, 50.0f);
    EXPECT_FLOAT_EQ(child7.center.y, 50.0f);
    EXPECT_FLOAT_EQ(child7.center.z, 50.0f);
    EXPECT_FLOAT_EQ(child7.halfSize, 50.0f);
    
    // Child octant 0 (-x, -y, -z) should have center at (-50, -50, -50)
    OctreeBounds child0 = parent.getChildBounds(0);
    EXPECT_FLOAT_EQ(child0.center.x, -50.0f);
    EXPECT_FLOAT_EQ(child0.center.y, -50.0f);
    EXPECT_FLOAT_EQ(child0.center.z, -50.0f);
    EXPECT_FLOAT_EQ(child0.halfSize, 50.0f);
}

// ============================================================
// Octree Manager Tests
// ============================================================

class OctreeTest : public ::testing::Test {
protected:
    Octree tree;
    OctreeTest() : tree(0.5f) {}
};

TEST_F(OctreeTest, EmptyTree) {
    std::vector<OctreeBody> bodies;
    tree.build(bodies);
    
    EXPECT_FALSE(tree.isBuilt());
    
    glm::vec3 acc = tree.computeAcceleration(glm::vec3(0.0f), G);
    EXPECT_FLOAT_EQ(acc.x, 0.0f);
    EXPECT_FLOAT_EQ(acc.y, 0.0f);
    EXPECT_FLOAT_EQ(acc.z, 0.0f);
}

TEST_F(OctreeTest, SingleBody) {
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f), 1e24f, "central")
    };
    tree.build(bodies);
    
    EXPECT_TRUE(tree.isBuilt());
    EXPECT_EQ(tree.getBodyCount(), 1);
    
    // Calculate acceleration at a known distance
    glm::vec3 testPos(1e9f, 0.0f, 0.0f);  // 1 billion meters away
    glm::vec3 acc = tree.computeAcceleration(testPos, G);
    
    // Expected: a = G * M / r^2, pointing towards the body (negative x direction)
    float expectedMag = G * 1e24f / (1e9f * 1e9f);
    EXPECT_NEAR(acc.x, -expectedMag, expectedMag * 1e-3f);
    EXPECT_NEAR(acc.y, 0.0f, 1e-10f);
    EXPECT_NEAR(acc.z, 0.0f, 1e-10f);
}

// ============================================================
// Barnes-Hut vs Direct Summation Comparison Tests
// ============================================================

class BarnesHutAccuracyTest : public ::testing::Test {
protected:
    /**
     * Compare Barnes-Hut acceleration with direct summation.
     * @return Relative error (0 to 1)
     */
    float compareAccuracy(const glm::vec3& testPos, 
                           const std::vector<OctreeBody>& bodies,
                           float theta) {
        // Direct summation (ground truth)
        glm::vec3 directAcc = directAcceleration(testPos, bodies, G);
        
        // Barnes-Hut
        Octree tree(theta);
        tree.build(bodies);
        glm::vec3 bhAcc = tree.computeAcceleration(testPos, G);
        
        float directMag = glm::length(directAcc);
        if (directMag < 1e-20f) return 0.0f;
        
        float errorMag = glm::length(bhAcc - directAcc);
        return errorMag / directMag;
    }
};

TEST_F(BarnesHutAccuracyTest, TwoBody_ExactMatch) {
    // With only two bodies, Barnes-Hut should give exact results
    // regardless of theta (no grouping possible)
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f), 1e24f, "body1"),
        OctreeBody(glm::vec3(1e10f, 0.0f, 0.0f), 1e22f, "body2")
    };
    
    glm::vec3 testPos(5e9f, 1e9f, 0.0f);
    float error = compareAccuracy(testPos, bodies, 0.5f);
    EXPECT_LT(error, 1e-3f) << "Two-body error should be negligible";
}

TEST_F(BarnesHutAccuracyTest, SolarSystem_RealisticSetup) {
    // Simulate a simplified solar system
    // Test that Barnes-Hut gives reasonable results for a rocket near Earth
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f),                              1.989e30f, "sun"),
        OctreeBody(glm::vec3(1.496e11f, 0.0f, 0.0f),           5.972e24f, "earth"),
        OctreeBody(glm::vec3(1.496e11f + 3.844e8f, 0.0f, 0.0f), 7.342e22f, "moon"),
        OctreeBody(glm::vec3(2.279e11f, 0.0f, 0.0f),           6.417e23f, "mars"),
        OctreeBody(glm::vec3(7.783e11f, 0.0f, 0.0f),           1.898e27f, "jupiter"),
        OctreeBody(glm::vec3(1.434e12f, 0.0f, 0.0f),           5.683e26f, "saturn"),
    };
    
    // Rocket position: 400 km above Earth's surface
    glm::vec3 rocketPos = glm::vec3(1.496e11f, 6.771e6f, 0.0f);
    
    // Compare with different theta values
    float error_theta_0 = compareAccuracy(rocketPos, bodies, 0.0f);   // Exact
    float error_theta_05 = compareAccuracy(rocketPos, bodies, 0.5f);
    float error_theta_10 = compareAccuracy(rocketPos, bodies, 1.0f);
    
    // theta=0 should be exact (within floating point)
    EXPECT_LT(error_theta_0, 1e-4f) << "theta=0 should be near-exact";
    
    // theta=0.5 should be quite accurate
    EXPECT_LT(error_theta_05, 0.01f) << "theta=0.5 error: " << error_theta_05;
    
    // theta=1.0 can be less accurate but still reasonable
    EXPECT_LT(error_theta_10, 0.1f) << "theta=1.0 error: " << error_theta_10;
    
    // Higher theta should have higher (or equal) error
    EXPECT_LE(error_theta_0, error_theta_05 + 1e-6f);
}

TEST_F(BarnesHutAccuracyTest, SolarSystem_AllPlanets) {
    // Full solar system with all 8 planets + Moon
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f),                              1.989e30f, "sun"),
        OctreeBody(glm::vec3(5.791e10f, 0.0f, 0.0f),            3.301e23f, "mercury"),
        OctreeBody(glm::vec3(1.082e11f, 0.0f, 0.0f),            4.867e24f, "venus"),
        OctreeBody(glm::vec3(1.496e11f, 0.0f, 0.0f),            5.972e24f, "earth"),
        OctreeBody(glm::vec3(1.496e11f, 3.844e8f, 0.0f),        7.342e22f, "moon"),
        OctreeBody(glm::vec3(2.279e11f, 0.0f, 0.0f),            6.417e23f, "mars"),
        OctreeBody(glm::vec3(7.783e11f, 0.0f, 0.0f),            1.898e27f, "jupiter"),
        OctreeBody(glm::vec3(1.434e12f, 0.0f, 0.0f),            5.683e26f, "saturn"),
        OctreeBody(glm::vec3(2.871e12f, 0.0f, 0.0f),            8.681e25f, "uranus"),
        OctreeBody(glm::vec3(4.495e12f, 0.0f, 0.0f),            1.024e26f, "neptune"),
    };
    
    // Rocket near Earth (Low Earth Orbit)
    glm::vec3 rocketPos = glm::vec3(1.496e11f + 6.771e6f, 0.0f, 0.0f);
    
    float error = compareAccuracy(rocketPos, bodies, 0.5f);
    EXPECT_LT(error, 0.02f) << "Full solar system error with theta=0.5: " << error;
    
    // Rocket near Mars
    glm::vec3 rocketPosMars = glm::vec3(2.279e11f + 3.5e6f, 0.0f, 0.0f);
    float errorMars = compareAccuracy(rocketPosMars, bodies, 0.5f);
    EXPECT_LT(errorMars, 0.02f) << "Near Mars error with theta=0.5: " << errorMars;
}

TEST_F(BarnesHutAccuracyTest, ThetaZero_MatchesDirect) {
    // theta=0 forces full tree traversal, should match direct summation exactly
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f), 1e30f, "a"),
        OctreeBody(glm::vec3(1e11f, 0.0f, 0.0f), 1e24f, "b"),
        OctreeBody(glm::vec3(0.0f, 1e11f, 0.0f), 1e24f, "c"),
        OctreeBody(glm::vec3(0.0f, 0.0f, 1e11f), 1e24f, "d"),
        OctreeBody(glm::vec3(-1e11f, 0.0f, 0.0f), 1e25f, "e"),
    };
    
    glm::vec3 testPos(5e10f, 5e10f, 0.0f);
    float error = compareAccuracy(testPos, bodies, 0.0f);
    EXPECT_LT(error, 1e-4f) << "theta=0 should match direct summation";
}

// ============================================================
// Performance Comparison Test
// ============================================================

TEST(OctreePerformanceTest, DirectVsBarnesHut) {
    // Generate bodies: simulate a solar system with additional small bodies
    std::vector<OctreeBody> bodies;
    
    // Sun
    bodies.emplace_back(glm::vec3(0.0f), 1.989e30f, "sun");
    
    // Planets (simplified positions along x-axis)
    bodies.emplace_back(glm::vec3(5.791e10f, 0.0f, 0.0f), 3.301e23f, "mercury");
    bodies.emplace_back(glm::vec3(1.082e11f, 0.0f, 0.0f), 4.867e24f, "venus");
    bodies.emplace_back(glm::vec3(1.496e11f, 0.0f, 0.0f), 5.972e24f, "earth");
    bodies.emplace_back(glm::vec3(2.279e11f, 0.0f, 0.0f), 6.417e23f, "mars");
    bodies.emplace_back(glm::vec3(7.783e11f, 0.0f, 0.0f), 1.898e27f, "jupiter");
    bodies.emplace_back(glm::vec3(1.434e12f, 0.0f, 0.0f), 5.683e26f, "saturn");
    bodies.emplace_back(glm::vec3(2.871e12f, 0.0f, 0.0f), 8.681e25f, "uranus");
    bodies.emplace_back(glm::vec3(4.495e12f, 0.0f, 0.0f), 1.024e26f, "neptune");
    
    // Add some asteroids for a more realistic test
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist_r(3.0e11f, 5.0e11f);  // Asteroid belt
    std::uniform_real_distribution<float> dist_angle(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> dist_mass(1e15f, 1e20f);
    
    for (int i = 0; i < 41; ++i) {
        float r = dist_r(rng);
        float angle = dist_angle(rng);
        float z = (dist_r(rng) - 4e11f) * 0.01f;  // Slight z variation
        bodies.emplace_back(
            glm::vec3(r * std::cos(angle), r * std::sin(angle), z),
            dist_mass(rng),
            "asteroid_" + std::to_string(i)
        );
    }
    
    int n = bodies.size();
    glm::vec3 testPos(1.496e11f + 6.771e6f, 0.0f, 0.0f);  // Near Earth
    
    // Benchmark: Direct summation
    auto t1 = std::chrono::high_resolution_clock::now();
    const int iterations = 1000;
    glm::vec3 directResult(0.0f);
    for (int i = 0; i < iterations; ++i) {
        directResult = directAcceleration(testPos, bodies, G);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double directTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (double)iterations;
    
    // Benchmark: Barnes-Hut (includes tree building)
    auto t3 = std::chrono::high_resolution_clock::now();
    glm::vec3 bhResult(0.0f);
    Octree tree(0.5f);
    for (int i = 0; i < iterations; ++i) {
        tree.build(bodies);
        bhResult = tree.computeAcceleration(testPos, G);
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    double bhTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / (double)iterations;
    
    // Print results
    float error = glm::length(bhResult - directResult) / glm::length(directResult);
    
    std::cout << "\n=== Barnes-Hut vs Direct Summation ===" << std::endl;
    std::cout << "Bodies: " << n << std::endl;
    std::cout << "Tree nodes: " << tree.getNodeCount() << std::endl;
    std::cout << "Direct acceleration: " << glm::to_string(directResult) << std::endl;
    std::cout << "Barnes-Hut acceleration: " << glm::to_string(bhResult) << std::endl;
    std::cout << "Relative error: " << error * 100.0f << "%" << std::endl;
    std::cout << "Direct time: " << directTimeUs << " us/iteration" << std::endl;
    std::cout << "Barnes-Hut time: " << bhTimeUs << " us/iteration" << std::endl;
    std::cout << "Speedup: " << directTimeUs / bhTimeUs << "x" << std::endl;
    std::cout << "======================================\n" << std::endl;
    
    // Verify accuracy is acceptable
    EXPECT_LT(error, 0.05f) << "Relative error should be < 5%";
}

// ============================================================
// Integration Test: Verify acceleration matches existing implementation
// ============================================================

TEST(OctreeIntegrationTest, MatchesProjectPhysics) {
    // Recreate the exact solar system setup from simulation.cpp
    // and verify Barnes-Hut gives the same acceleration as direct summation
    
    // Using approximate values from config defaults
    float G_val = 6.674e-11f;
    float earth_orbit_radius = 1.496e11f;
    float earth_mass = 5.972e24f;
    float sun_mass = 1.989e30f;
    float moon_distance = 3.844e8f;
    float moon_mass = 7.342e22f;
    
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f), sun_mass, "sun"),
        OctreeBody(glm::vec3(earth_orbit_radius, 0.0f, 0.0f), earth_mass, "earth"),
        OctreeBody(glm::vec3(earth_orbit_radius, moon_distance, 0.0f), moon_mass, "moon"),
    };
    
    // Rocket at Earth's surface (on top of Earth along y-axis relative to Earth)
    glm::vec3 rocketPos(earth_orbit_radius, 6.371e6f, 0.0f);
    
    // Direct calculation
    glm::vec3 directAcc = directAcceleration(rocketPos, bodies, G_val);
    
    // Barnes-Hut calculation with theta=0 (should be exact)
    Octree tree(0.0f);
    tree.build(bodies);
    glm::vec3 bhAcc = tree.computeAcceleration(rocketPos, G_val);
    
    // Compare
    float error = glm::length(bhAcc - directAcc) / glm::length(directAcc);
    EXPECT_LT(error, 1e-4f) << "Integration test: theta=0 should match direct\n"
                             << "Direct: " << glm::to_string(directAcc) << "\n"
                             << "BH:     " << glm::to_string(bhAcc);
    
    // Also test with theta=0.5
    Octree tree05(0.5f);
    tree05.build(bodies);
    glm::vec3 bhAcc05 = tree05.computeAcceleration(rocketPos, G_val);
    
    float error05 = glm::length(bhAcc05 - directAcc) / glm::length(directAcc);
    EXPECT_LT(error05, 0.02f) << "Integration test: theta=0.5 should be accurate\n"
                              << "Direct: " << glm::to_string(directAcc) << "\n"
                              << "BH:     " << glm::to_string(bhAcc05);
}

// ============================================================
// Edge Case Tests
// ============================================================

TEST(OctreeEdgeTest, VeryLargeMassRange) {
    // Sun mass vs asteroid mass: factor of ~10^15
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f), 1.989e30f, "sun"),
        OctreeBody(glm::vec3(1e11f, 0.0f, 0.0f), 1e15f, "asteroid"),
    };
    
    glm::vec3 testPos(5e10f, 0.0f, 0.0f);
    
    // Should not crash or produce NaN
    Octree tree(0.5f);
    tree.build(bodies);
    glm::vec3 acc = tree.computeAcceleration(testPos, G);
    
    EXPECT_FALSE(std::isnan(acc.x));
    EXPECT_FALSE(std::isnan(acc.y));
    EXPECT_FALSE(std::isnan(acc.z));
    EXPECT_FALSE(std::isinf(acc.x));
}

TEST(OctreeEdgeTest, AllBodiesInSameOctant) {
    // All bodies clustered in one octant
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(1.0f, 1.0f, 1.0f), 100.0f),
        OctreeBody(glm::vec3(2.0f, 1.0f, 1.0f), 100.0f),
        OctreeBody(glm::vec3(1.0f, 2.0f, 1.0f), 100.0f),
        OctreeBody(glm::vec3(1.0f, 1.0f, 2.0f), 100.0f),
    };
    
    Octree tree(0.5f);
    tree.build(bodies);
    
    EXPECT_EQ(tree.getBodyCount(), 4);
    
    glm::vec3 acc = tree.computeAcceleration(glm::vec3(-10.0f, 0.0f, 0.0f), G);
    EXPECT_FALSE(std::isnan(acc.x));
}

TEST(OctreeEdgeTest, QueryPositionAtBodyLocation) {
    // Query acceleration at the exact position of a body (should skip self)
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::vec3(0.0f), 1e24f, "central"),
        OctreeBody(glm::vec3(1e9f, 0.0f, 0.0f), 1e20f, "other"),
    };
    
    Octree tree(0.5f);
    tree.build(bodies);
    
    // Query at the position of "central" body
    glm::vec3 acc = tree.computeAcceleration(glm::vec3(0.0f), G);
    
    // Should get acceleration from "other" body only, no singularity
    EXPECT_FALSE(std::isnan(acc.x));
    EXPECT_FALSE(std::isinf(acc.x));
    // Should point towards "other" (positive x direction)
    EXPECT_GT(acc.x, 0.0f);
}
