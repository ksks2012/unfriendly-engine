#!/usr/bin/env python3
"""
Orbital Mechanics Verification Script
=====================================
This script verifies the orbital parameters used in the solar system simulation
against Kepler's third law and known astronomical data.

Usage:
    python3 verify/verify_orbits.py
"""

import math

# Physical constants
G = 6.674e-11  # Gravitational constant (m³/kg/s²)
M_SUN = 1.989e30  # Solar mass (kg)
AU = 149597870700.0  # Astronomical Unit in meters

# Planet data from config.h
# Format: (name, orbit_radius_m, orbital_velocity_m_s, expected_period_days, inclination_deg)
PLANETS = {
    'Mercury': {
        'orbit_radius': 57909050000.0,
        'orbital_velocity': 47362.0,
        'expected_period_days': 88,
        'inclination': 7.005,
        'radius': 2439700.0,
        'mass': 3.3011e23,
    },
    'Venus': {
        'orbit_radius': 108208000000.0,
        'orbital_velocity': 35020.0,
        'expected_period_days': 225,
        'inclination': 3.395,
        'radius': 6051800.0,
        'mass': 4.8675e24,
    },
    'Earth': {
        'orbit_radius': 149597870700.0,
        'orbital_velocity': 29780.0,
        'expected_period_days': 365,
        'inclination': 0.0,
        'radius': 6371000.0,
        'mass': 5.972e24,
    },
    'Mars': {
        'orbit_radius': 227939200000.0,
        'orbital_velocity': 24077.0,
        'expected_period_days': 687,
        'inclination': 1.850,
        'radius': 3389500.0,
        'mass': 6.4171e23,
    },
    'Jupiter': {
        'orbit_radius': 778.57e9,
        'orbital_velocity': 13070.0,
        'expected_period_days': 4333,
        'inclination': 1.303,
        'radius': 69911000.0,
        'mass': 1.8982e27,
    },
    'Saturn': {
        'orbit_radius': 1433.53e9,
        'orbital_velocity': 9680.0,
        'expected_period_days': 10759,
        'inclination': 2.485,
        'radius': 58232000.0,
        'mass': 5.6834e26,
    },
    'Uranus': {
        'orbit_radius': 2872.46e9,
        'orbital_velocity': 6800.0,
        'expected_period_days': 30687,
        'inclination': 0.773,
        'radius': 25362000.0,
        'mass': 8.6810e25,
    },
    'Neptune': {
        'orbit_radius': 4495.06e9,
        'orbital_velocity': 5430.0,
        'expected_period_days': 60190,
        'inclination': 1.770,
        'radius': 24622000.0,
        'mass': 1.02413e26,
    },
}


def calculate_orbital_period(semi_major_axis_m):
    """Calculate orbital period using Kepler's third law: T = 2*pi*sqrt(a³/(G*M))"""
    T_seconds = 2 * math.pi * math.sqrt(semi_major_axis_m**3 / (G * M_SUN))
    return T_seconds / (24 * 3600)  # Convert to days


def calculate_orbital_velocity(semi_major_axis_m):
    """Calculate circular orbital velocity: v = sqrt(G*M/r)"""
    return math.sqrt(G * M_SUN / semi_major_axis_m)


def verify_orbital_periods():
    """Verify orbital periods against Kepler's third law"""
    print("\n" + "=" * 80)
    print("ORBITAL PERIOD VERIFICATION (Kepler's Third Law: T = 2π√(a³/GM))")
    print("=" * 80)
    print(f"{'Planet':<10} | {'Calculated':>12} | {'Expected':>12} | {'Error':>8} | {'Status':<6}")
    print("-" * 80)
    
    all_passed = True
    for name, data in PLANETS.items():
        calculated = calculate_orbital_period(data['orbit_radius'])
        expected = data['expected_period_days']
        error = abs(calculated - expected) / expected * 100
        status = "✓ PASS" if error < 2.0 else "✗ FAIL"
        if error >= 2.0:
            all_passed = False
        print(f"{name:<10} | {calculated:>10.1f} d | {expected:>10d} d | {error:>6.2f} % | {status}")
    
    return all_passed


def verify_orbital_velocities():
    """Verify orbital velocities against circular orbit formula"""
    print("\n" + "=" * 80)
    print("ORBITAL VELOCITY VERIFICATION (v = √(GM/r))")
    print("=" * 80)
    print(f"{'Planet':<10} | {'Calculated':>12} | {'Config':>12} | {'Error':>8} | {'Status':<6}")
    print("-" * 80)
    
    all_passed = True
    for name, data in PLANETS.items():
        calculated = calculate_orbital_velocity(data['orbit_radius'])
        config_value = data['orbital_velocity']
        error = abs(calculated - config_value) / config_value * 100
        status = "✓ PASS" if error < 2.0 else "✗ FAIL"
        if error >= 2.0:
            all_passed = False
        print(f"{name:<10} | {calculated:>10.0f} m/s | {config_value:>10.0f} m/s | {error:>6.2f} % | {status}")
    
    return all_passed


def verify_orbit_distances():
    """Verify orbit distances in AU"""
    print("\n" + "=" * 80)
    print("ORBITAL DISTANCE VERIFICATION (in Astronomical Units)")
    print("=" * 80)
    
    # Expected distances in AU (approximate)
    expected_au = {
        'Mercury': 0.387,
        'Venus': 0.723,
        'Earth': 1.000,
        'Mars': 1.524,
        'Jupiter': 5.204,
        'Saturn': 9.583,
        'Uranus': 19.19,
        'Neptune': 30.07,
    }
    
    print(f"{'Planet':<10} | {'Distance (m)':>18} | {'AU':>8} | {'Expected AU':>12}")
    print("-" * 80)
    
    for name, data in PLANETS.items():
        au = data['orbit_radius'] / AU
        expected = expected_au.get(name, 0)
        print(f"{name:<10} | {data['orbit_radius']:>18.2e} | {au:>8.3f} | {expected:>12.3f}")


def print_summary():
    """Print a summary of all planets"""
    print("\n" + "=" * 80)
    print("SOLAR SYSTEM SUMMARY")
    print("=" * 80)
    print(f"{'Planet':<10} | {'Radius (km)':>12} | {'Mass (kg)':>12} | {'Orbit (AU)':>10} | {'Incl. (°)':>9}")
    print("-" * 80)
    
    for name, data in PLANETS.items():
        radius_km = data['radius'] / 1000
        orbit_au = data['orbit_radius'] / AU
        print(f"{name:<10} | {radius_km:>12,.0f} | {data['mass']:>12.2e} | {orbit_au:>10.3f} | {data['inclination']:>9.3f}")


def main():
    print("=" * 80)
    print("ORBITAL MECHANICS VERIFICATION FOR SOLAR SYSTEM SIMULATION")
    print("=" * 80)
    print(f"Gravitational Constant G = {G} m³/kg/s²")
    print(f"Solar Mass M☉ = {M_SUN:.3e} kg")
    print(f"Astronomical Unit = {AU:,.0f} m")
    
    period_ok = verify_orbital_periods()
    velocity_ok = verify_orbital_velocities()
    verify_orbit_distances()
    print_summary()
    
    print("\n" + "=" * 80)
    print("VERIFICATION RESULT")
    print("=" * 80)
    if period_ok and velocity_ok:
        print("✓ All orbital parameters are within acceptable error margins (< 2%)")
        return 0
    else:
        print("✗ Some orbital parameters have errors exceeding 2%")
        return 1


if __name__ == "__main__":
    exit(main())
