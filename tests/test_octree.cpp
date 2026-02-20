#include "core/octree.h"

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <cmath>
#include <random>
#include <chrono>
#include <iostream>

// Gravitational constant used in the project
static constexpr double G = 6.674e-11;

// ============================================================
// Helper: Direct N-body force calculation for comparison
// ============================================================

/**
 * Compute gravitational acceleration on a body using direct summation (O(n^2)).
 * This serves as the ground truth for validating Barnes-Hut results.
 */
glm::dvec3 directAcceleration(const glm::dvec3& position, 
                              const std::vector<OctreeBody>& bodies,
                              double gravitationalConstant) {
    glm::dvec3 acc(0.0);
    
    for (const auto& body : bodies) {
        glm::dvec3 delta = body.position - position;
        double distSq = glm::dot(delta, delta);
        double dist = std::sqrt(distSq);
        if (dist > 1e-6) {
            double distCubed = distSq * dist;
            acc += (gravitationalConstant * body.mass / distCubed) * delta;
        }
    }
    return acc;
}

// ============================================================
// OctreeNode Unit Tests
// ============================================================

class OctreeNodeTest : public ::testing::Test {
protected:
    OctreeBounds defaultBounds = OctreeBounds(glm::dvec3(0.0), 100.0);
};

TEST_F(OctreeNodeTest, EmptyNode) {
    OctreeNode node(defaultBounds);
    EXPECT_TRUE(node.isEmpty());
    EXPECT_FALSE(node.isLeaf());
    EXPECT_FALSE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 0);
    EXPECT_DOUBLE_EQ(node.getTotalMass(), 0.0);
}

TEST_F(OctreeNodeTest, InsertSingleBody) {
    OctreeNode node(defaultBounds);
    OctreeBody body(glm::dvec3(10.0, 20.0, 30.0), 100.0, "test");
    
    node.insert(body);
    
    EXPECT_FALSE(node.isEmpty());
    EXPECT_TRUE(node.isLeaf());
    EXPECT_FALSE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 1);
    EXPECT_DOUBLE_EQ(node.getTotalMass(), 100.0);
    EXPECT_EQ(node.getCenterOfMass(), body.position);
}

TEST_F(OctreeNodeTest, InsertTwoBodies_Subdivides) {
    OctreeNode node(defaultBounds);
    OctreeBody body1(glm::dvec3(50.0, 50.0, 50.0), 100.0, "body1");
    OctreeBody body2(glm::dvec3(-50.0, -50.0, -50.0), 200.0, "body2");
    
    node.insert(body1);
    node.insert(body2);
    
    EXPECT_TRUE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 2);
    EXPECT_DOUBLE_EQ(node.getTotalMass(), 300.0);
    
    // Center of mass should be weighted average
    glm::dvec3 expectedCom = (100.0 * body1.position + 200.0 * body2.position) / 300.0;
    EXPECT_NEAR(node.getCenterOfMass().x, expectedCom.x, 1e-3);
    EXPECT_NEAR(node.getCenterOfMass().y, expectedCom.y, 1e-3);
    EXPECT_NEAR(node.getCenterOfMass().z, expectedCom.z, 1e-3);
}

TEST_F(OctreeNodeTest, InsertMultipleBodies) {
    OctreeNode node(defaultBounds);
    
    node.insert(OctreeBody(glm::dvec3(50.0, 50.0, 50.0), 100.0));
    node.insert(OctreeBody(glm::dvec3(-50.0, 50.0, 50.0), 100.0));
    node.insert(OctreeBody(glm::dvec3(50.0, -50.0, 50.0), 100.0));
    node.insert(OctreeBody(glm::dvec3(-50.0, -50.0, -50.0), 100.0));
    
    EXPECT_TRUE(node.isInternal());
    EXPECT_EQ(node.getBodyCount(), 4);
    EXPECT_DOUBLE_EQ(node.getTotalMass(), 400.0);
}

TEST_F(OctreeNodeTest, BodiesAtSamePosition_NoInfiniteRecursion) {
    OctreeNode node(defaultBounds);
    
    // Insert two bodies at the exact same position
    // Should not cause infinite recursion due to MAX_DEPTH limit
    node.insert(OctreeBody(glm::dvec3(10.0), 100.0, "a"));
    node.insert(OctreeBody(glm::dvec3(10.0), 200.0, "b"));
    
    EXPECT_EQ(node.getBodyCount(), 2);
    EXPECT_DOUBLE_EQ(node.getTotalMass(), 300.0);
}

TEST_F(OctreeNodeTest, NodeCount) {
    OctreeNode node(defaultBounds);
    
    // Empty node: just 1 (root)
    EXPECT_EQ(node.getNodeCount(), 1);
    
    // One body: still 1 (leaf)
    node.insert(OctreeBody(glm::dvec3(50.0), 100.0));
    EXPECT_EQ(node.getNodeCount(), 1);
    
    // Two bodies: root + 8 children = 9
    node.insert(OctreeBody(glm::dvec3(-50.0), 100.0));
    EXPECT_EQ(node.getNodeCount(), 9);
}

// ============================================================
// OctreeBounds Tests
// ============================================================

TEST(OctreeBoundsTest, Contains) {
    OctreeBounds bounds(glm::dvec3(0.0), 100.0);
    
    EXPECT_TRUE(bounds.contains(glm::dvec3(0.0)));
    EXPECT_TRUE(bounds.contains(glm::dvec3(50.0, 50.0, 50.0)));
    EXPECT_TRUE(bounds.contains(glm::dvec3(-99.0, -99.0, -99.0)));
    EXPECT_TRUE(bounds.contains(glm::dvec3(100.0, 100.0, 100.0)));  // On boundary
    
    EXPECT_FALSE(bounds.contains(glm::dvec3(101.0, 0.0, 0.0)));
    EXPECT_FALSE(bounds.contains(glm::dvec3(0.0, -101.0, 0.0)));
}

TEST(OctreeBoundsTest, ChildBounds) {
    OctreeBounds parent(glm::dvec3(0.0), 100.0);
    
    // Child octant 7 (+x, +y, +z) should have center at (50, 50, 50) with halfSize 50
    OctreeBounds child7 = parent.getChildBounds(7);
    EXPECT_DOUBLE_EQ(child7.center.x, 50.0);
    EXPECT_DOUBLE_EQ(child7.center.y, 50.0);
    EXPECT_DOUBLE_EQ(child7.center.z, 50.0);
    EXPECT_DOUBLE_EQ(child7.halfSize, 50.0);
    
    // Child octant 0 (-x, -y, -z) should have center at (-50, -50, -50)
    OctreeBounds child0 = parent.getChildBounds(0);
    EXPECT_DOUBLE_EQ(child0.center.x, -50.0);
    EXPECT_DOUBLE_EQ(child0.center.y, -50.0);
    EXPECT_DOUBLE_EQ(child0.center.z, -50.0);
    EXPECT_DOUBLE_EQ(child0.halfSize, 50.0);
}

// ============================================================
// Octree Manager Tests
// ============================================================

class OctreeTest : public ::testing::Test {
protected:
    Octree tree;
    OctreeTest() : tree(0.5) {}
};

TEST_F(OctreeTest, EmptyTree) {
    std::vector<OctreeBody> bodies;
    tree.build(bodies);
    
    EXPECT_FALSE(tree.isBuilt());
    
    glm::dvec3 acc = tree.computeAcceleration(glm::dvec3(0.0), G);
    EXPECT_DOUBLE_EQ(acc.x, 0.0);
    EXPECT_DOUBLE_EQ(acc.y, 0.0);
    EXPECT_DOUBLE_EQ(acc.z, 0.0);
}

TEST_F(OctreeTest, SingleBody) {
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0), 1e24, "central")
    };
    tree.build(bodies);
    
    EXPECT_TRUE(tree.isBuilt());
    EXPECT_EQ(tree.getBodyCount(), 1);
    
    // Calculate acceleration at a known distance
    glm::dvec3 testPos(1e9, 0.0, 0.0);  // 1 billion meters away
    glm::dvec3 acc = tree.computeAcceleration(testPos, G);
    
    // Expected: a = G * M / r^2, pointing towards the body (negative x direction)
    double expectedMag = G * 1e24 / (1e9 * 1e9);
    EXPECT_NEAR(acc.x, -expectedMag, expectedMag * 1e-3);
    EXPECT_NEAR(acc.y, 0.0, 1e-10);
    EXPECT_NEAR(acc.z, 0.0, 1e-10);
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
    double compareAccuracy(const glm::dvec3& testPos, 
                           const std::vector<OctreeBody>& bodies,
                           double theta) {
        // Direct summation (ground truth)
        glm::dvec3 directAcc = directAcceleration(testPos, bodies, G);
        
        // Barnes-Hut
        Octree tree(theta);
        tree.build(bodies);
        glm::dvec3 bhAcc = tree.computeAcceleration(testPos, G);
        
        double directMag = glm::length(directAcc);
        if (directMag < 1e-20) return 0.0;
        
        double errorMag = glm::length(bhAcc - directAcc);
        return errorMag / directMag;
    }
};

TEST_F(BarnesHutAccuracyTest, TwoBody_ExactMatch) {
    // With only two bodies, Barnes-Hut should give exact results
    // regardless of theta (no grouping possible)
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0), 1e24, "body1"),
        OctreeBody(glm::dvec3(1e10, 0.0, 0.0), 1e22, "body2")
    };
    
    glm::dvec3 testPos(5e9, 1e9, 0.0);
    double error = compareAccuracy(testPos, bodies, 0.5);
    EXPECT_LT(error, 1e-3) << "Two-body error should be negligible";
}

TEST_F(BarnesHutAccuracyTest, SolarSystem_RealisticSetup) {
    // Simulate a simplified solar system
    // Test that Barnes-Hut gives reasonable results for a rocket near Earth
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0),                              1.989e30, "sun"),
        OctreeBody(glm::dvec3(1.496e11, 0.0, 0.0),              5.972e24, "earth"),
        OctreeBody(glm::dvec3(1.496e11 + 3.844e8, 0.0, 0.0),    7.342e22, "moon"),
        OctreeBody(glm::dvec3(2.279e11, 0.0, 0.0),              6.417e23, "mars"),
        OctreeBody(glm::dvec3(7.783e11, 0.0, 0.0),              1.898e27, "jupiter"),
        OctreeBody(glm::dvec3(1.434e12, 0.0, 0.0),              5.683e26, "saturn"),
    };
    
    // Rocket position: 400 km above Earth's surface
    glm::dvec3 rocketPos = glm::dvec3(1.496e11, 6.771e6, 0.0);
    
    // Compare with different theta values
    double error_theta_0 = compareAccuracy(rocketPos, bodies, 0.0);   // Exact
    double error_theta_05 = compareAccuracy(rocketPos, bodies, 0.5);
    double error_theta_10 = compareAccuracy(rocketPos, bodies, 1.0);
    
    // theta=0 should be exact (within floating point)
    EXPECT_LT(error_theta_0, 1e-4) << "theta=0 should be near-exact";
    
    // theta=0.5 should be quite accurate
    EXPECT_LT(error_theta_05, 0.01) << "theta=0.5 error: " << error_theta_05;
    
    // theta=1.0 can be less accurate but still reasonable
    EXPECT_LT(error_theta_10, 0.1) << "theta=1.0 error: " << error_theta_10;
    
    // Higher theta should have higher (or equal) error
    EXPECT_LE(error_theta_0, error_theta_05 + 1e-6);
}

TEST_F(BarnesHutAccuracyTest, SolarSystem_AllPlanets) {
    // Full solar system with all 8 planets + Moon
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0),                              1.989e30, "sun"),
        OctreeBody(glm::dvec3(5.791e10, 0.0, 0.0),              3.301e23, "mercury"),
        OctreeBody(glm::dvec3(1.082e11, 0.0, 0.0),              4.867e24, "venus"),
        OctreeBody(glm::dvec3(1.496e11, 0.0, 0.0),              5.972e24, "earth"),
        OctreeBody(glm::dvec3(1.496e11, 3.844e8, 0.0),          7.342e22, "moon"),
        OctreeBody(glm::dvec3(2.279e11, 0.0, 0.0),              6.417e23, "mars"),
        OctreeBody(glm::dvec3(7.783e11, 0.0, 0.0),              1.898e27, "jupiter"),
        OctreeBody(glm::dvec3(1.434e12, 0.0, 0.0),              5.683e26, "saturn"),
        OctreeBody(glm::dvec3(2.871e12, 0.0, 0.0),              8.681e25, "uranus"),
        OctreeBody(glm::dvec3(4.495e12, 0.0, 0.0),              1.024e26, "neptune"),
    };
    
    // Rocket near Earth (Low Earth Orbit)
    glm::dvec3 rocketPos = glm::dvec3(1.496e11 + 6.771e6, 0.0, 0.0);
    
    double error = compareAccuracy(rocketPos, bodies, 0.5);
    EXPECT_LT(error, 0.02) << "Full solar system error with theta=0.5: " << error;
    
    // Rocket near Mars
    glm::dvec3 rocketPosMars = glm::dvec3(2.279e11 + 3.5e6, 0.0, 0.0);
    double errorMars = compareAccuracy(rocketPosMars, bodies, 0.5);
    EXPECT_LT(errorMars, 0.02) << "Near Mars error with theta=0.5: " << errorMars;
}

TEST_F(BarnesHutAccuracyTest, ThetaZero_MatchesDirect) {
    // theta=0 forces full tree traversal, should match direct summation exactly
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0), 1e30, "a"),
        OctreeBody(glm::dvec3(1e11, 0.0, 0.0), 1e24, "b"),
        OctreeBody(glm::dvec3(0.0, 1e11, 0.0), 1e24, "c"),
        OctreeBody(glm::dvec3(0.0, 0.0, 1e11), 1e24, "d"),
        OctreeBody(glm::dvec3(-1e11, 0.0, 0.0), 1e25, "e"),
    };
    
    glm::dvec3 testPos(5e10, 5e10, 0.0);
    double error = compareAccuracy(testPos, bodies, 0.0);
    EXPECT_LT(error, 1e-4) << "theta=0 should match direct summation";
}

// ============================================================
// Performance Comparison Test
// ============================================================

TEST(OctreePerformanceTest, DirectVsBarnesHut) {
    // Generate bodies: simulate a solar system with additional small bodies
    std::vector<OctreeBody> bodies;
    
    // Sun
    bodies.emplace_back(glm::dvec3(0.0), 1.989e30, "sun");
    
    // Planets (simplified positions along x-axis)
    bodies.emplace_back(glm::dvec3(5.791e10, 0.0, 0.0), 3.301e23, "mercury");
    bodies.emplace_back(glm::dvec3(1.082e11, 0.0, 0.0), 4.867e24, "venus");
    bodies.emplace_back(glm::dvec3(1.496e11, 0.0, 0.0), 5.972e24, "earth");
    bodies.emplace_back(glm::dvec3(2.279e11, 0.0, 0.0), 6.417e23, "mars");
    bodies.emplace_back(glm::dvec3(7.783e11, 0.0, 0.0), 1.898e27, "jupiter");
    bodies.emplace_back(glm::dvec3(1.434e12, 0.0, 0.0), 5.683e26, "saturn");
    bodies.emplace_back(glm::dvec3(2.871e12, 0.0, 0.0), 8.681e25, "uranus");
    bodies.emplace_back(glm::dvec3(4.495e12, 0.0, 0.0), 1.024e26, "neptune");
    
    // Add some asteroids for a more realistic test
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist_r(3.0e11, 5.0e11);  // Asteroid belt
    std::uniform_real_distribution<double> dist_angle(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<double> dist_mass(1e15, 1e20);
    
    for (int i = 0; i < 41; ++i) {
        double r = dist_r(rng);
        double angle = dist_angle(rng);
        double z = (dist_r(rng) - 4e11) * 0.01;  // Slight z variation
        bodies.emplace_back(
            glm::dvec3(r * std::cos(angle), r * std::sin(angle), z),
            dist_mass(rng),
            "asteroid_" + std::to_string(i)
        );
    }
    
    int n = bodies.size();
    glm::dvec3 testPos(1.496e11 + 6.771e6, 0.0, 0.0);  // Near Earth
    
    // Benchmark: Direct summation
    auto t1 = std::chrono::high_resolution_clock::now();
    const int iterations = 1000;
    glm::dvec3 directResult(0.0);
    for (int i = 0; i < iterations; ++i) {
        directResult = directAcceleration(testPos, bodies, G);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double directTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (double)iterations;
    
    // Benchmark: Barnes-Hut (includes tree building)
    auto t3 = std::chrono::high_resolution_clock::now();
    glm::dvec3 bhResult(0.0);
    Octree tree(0.5);
    for (int i = 0; i < iterations; ++i) {
        tree.build(bodies);
        bhResult = tree.computeAcceleration(testPos, G);
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    double bhTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / (double)iterations;
    
    // Print results
    double error = glm::length(bhResult - directResult) / glm::length(directResult);
    
    std::cout << "\n=== Barnes-Hut vs Direct Summation ===" << std::endl;
    std::cout << "Bodies: " << n << std::endl;
    std::cout << "Tree nodes: " << tree.getNodeCount() << std::endl;
    std::cout << "Direct acceleration: " << glm::to_string(directResult) << std::endl;
    std::cout << "Barnes-Hut acceleration: " << glm::to_string(bhResult) << std::endl;
    std::cout << "Relative error: " << error * 100.0 << "%" << std::endl;
    std::cout << "Direct time: " << directTimeUs << " us/iteration" << std::endl;
    std::cout << "Barnes-Hut time: " << bhTimeUs << " us/iteration" << std::endl;
    std::cout << "Speedup: " << directTimeUs / bhTimeUs << "x" << std::endl;
    std::cout << "======================================\n" << std::endl;
    
    // Verify accuracy is acceptable
    EXPECT_LT(error, 0.05) << "Relative error should be < 5%";
}

// ============================================================
// Integration Test: Verify acceleration matches existing implementation
// ============================================================

TEST(OctreeIntegrationTest, MatchesProjectPhysics) {
    // Recreate the exact solar system setup from simulation.cpp
    // and verify Barnes-Hut gives the same acceleration as direct summation
    
    // Using approximate values from config defaults
    double G_val = 6.674e-11;
    double earth_orbit_radius = 1.496e11;
    double earth_mass = 5.972e24;
    double sun_mass = 1.989e30;
    double moon_distance = 3.844e8;
    double moon_mass = 7.342e22;
    
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0), sun_mass, "sun"),
        OctreeBody(glm::dvec3(earth_orbit_radius, 0.0, 0.0), earth_mass, "earth"),
        OctreeBody(glm::dvec3(earth_orbit_radius, moon_distance, 0.0), moon_mass, "moon"),
    };
    
    // Rocket at Earth's surface (on top of Earth along y-axis relative to Earth)
    glm::dvec3 rocketPos(earth_orbit_radius, 6.371e6, 0.0);
    
    // Direct calculation
    glm::dvec3 directAcc = directAcceleration(rocketPos, bodies, G_val);
    
    // Barnes-Hut calculation with theta=0 (should be exact)
    Octree tree(0.0);
    tree.build(bodies);
    glm::dvec3 bhAcc = tree.computeAcceleration(rocketPos, G_val);
    
    // Compare
    double error = glm::length(bhAcc - directAcc) / glm::length(directAcc);
    EXPECT_LT(error, 1e-4) << "Integration test: theta=0 should match direct\n"
                             << "Direct: " << glm::to_string(directAcc) << "\n"
                             << "BH:     " << glm::to_string(bhAcc);
    
    // Also test with theta=0.5
    Octree tree05(0.5);
    tree05.build(bodies);
    glm::dvec3 bhAcc05 = tree05.computeAcceleration(rocketPos, G_val);
    
    double error05 = glm::length(bhAcc05 - directAcc) / glm::length(directAcc);
    EXPECT_LT(error05, 0.02) << "Integration test: theta=0.5 should be accurate\n"
                              << "Direct: " << glm::to_string(directAcc) << "\n"
                              << "BH:     " << glm::to_string(bhAcc05);
}

// ============================================================
// Edge Case Tests
// ============================================================

TEST(OctreeEdgeTest, VeryLargeMassRange) {
    // Sun mass vs asteroid mass: factor of ~10^15
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0), 1.989e30, "sun"),
        OctreeBody(glm::dvec3(1e11, 0.0, 0.0), 1e15, "asteroid"),
    };
    
    glm::dvec3 testPos(5e10, 0.0, 0.0);
    
    // Should not crash or produce NaN
    Octree tree(0.5);
    tree.build(bodies);
    glm::dvec3 acc = tree.computeAcceleration(testPos, G);
    
    EXPECT_FALSE(std::isnan(acc.x));
    EXPECT_FALSE(std::isnan(acc.y));
    EXPECT_FALSE(std::isnan(acc.z));
    EXPECT_FALSE(std::isinf(acc.x));
}

TEST(OctreeEdgeTest, AllBodiesInSameOctant) {
    // All bodies clustered in one octant
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(1.0, 1.0, 1.0), 100.0),
        OctreeBody(glm::dvec3(2.0, 1.0, 1.0), 100.0),
        OctreeBody(glm::dvec3(1.0, 2.0, 1.0), 100.0),
        OctreeBody(glm::dvec3(1.0, 1.0, 2.0), 100.0),
    };
    
    Octree tree(0.5);
    tree.build(bodies);
    
    EXPECT_EQ(tree.getBodyCount(), 4);
    
    glm::dvec3 acc = tree.computeAcceleration(glm::dvec3(-10.0, 0.0, 0.0), G);
    EXPECT_FALSE(std::isnan(acc.x));
}

TEST(OctreeEdgeTest, QueryPositionAtBodyLocation) {
    // Query acceleration at the exact position of a body (should skip self)
    std::vector<OctreeBody> bodies = {
        OctreeBody(glm::dvec3(0.0), 1e24, "central"),
        OctreeBody(glm::dvec3(1e9, 0.0, 0.0), 1e20, "other"),
    };
    
    Octree tree(0.5);
    tree.build(bodies);
    
    // Query at the position of "central" body
    glm::dvec3 acc = tree.computeAcceleration(glm::dvec3(0.0), G);
    
    // Should get acceleration from "other" body only, no singularity
    EXPECT_FALSE(std::isnan(acc.x));
    EXPECT_FALSE(std::isinf(acc.x));
    // Should point towards "other" (positive x direction)
    EXPECT_GT(acc.x, 0.0);
}
