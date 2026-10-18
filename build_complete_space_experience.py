"""
========================================================================
 SPACE ODYSSEY — Professional Level Builder v2.0
 Unreal Engine 5.8 Python Editor Script
 
 Builds the complete game level with:
   - Custom PBR materials (MaterialEditingLibrary)
   - Earth surface with launch pad structures
   - Atmospheric system (sky, clouds, fog)
   - Moon at correct position with surface material
   - Starfield with individual emissive stars
   - Player spaceship with cockpit frame
   - Blueprint architecture (GameMode, HUD)
========================================================================
"""
import unreal
import math
import random

print("=" * 70)
print("  SPACE ODYSSEY — Professional Level Builder v2.0")
print("=" * 70)

# ============================================================
# CONFIGURATION
# ============================================================
MAP_PATH = "/Game/Maps/SpaceFlight_Cinematic"
MATERIALS_PATH = "/Game/Materials"
BLUEPRINTS_PATH = "/Game/Blueprints"

MOON_ALTITUDE = 8000000.0      # 80 km above ground
MOON_SCALE = 8000.0            # ~8 km diameter
STARFIELD_RADIUS = 800000.0    # 8 km radius background sphere
STAR_COUNT = 35
STAR_MIN_DIST = 2000000.0      # 20 km
STAR_MAX_DIST = 6000000.0      # 60 km
PLAYER_START_Z = 2500.0        # 25 m above ground
PAWN_MAX_SPEED = 100000.0      # 1 km/s

random.seed(42)

# ============================================================
# MATERIAL CACHE
# ============================================================
materials_cache = {}
HAS_MATERIAL_EDITING = False

# ============================================================
# HELPER: Material Creation via MaterialEditingLibrary
# ============================================================
def create_material(name, base_color=None, roughness=0.5, metallic=0.0,
                    emissive_color=None, emissive_strength=1.0,
                    two_sided=False, unlit=False):
    """Create a PBR material with the specified properties."""
    global HAS_MATERIAL_EDITING
    if not HAS_MATERIAL_EDITING:
        return None

    full_path = f"{MATERIALS_PATH}/{name}"

    # Delete existing asset if present
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        unreal.EditorAssetLibrary.delete_asset(full_path)

    # Create material asset
    factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = asset_tools.create_asset(name, MATERIALS_PATH, None, factory)

    if not mat:
        print(f"    WARNING: Failed to create material {name}")
        return None

    mel = unreal.MaterialEditingLibrary
    y_pos = 0

    # -- Base Color --
    if base_color and not unlit:
        node = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -400, y_pos)
        node.set_editor_property("constant", base_color)
        mel.connect_material_property(
            node, "", unreal.MaterialProperty.MP_BASE_COLOR)
        y_pos += 200

    # -- Roughness --
    if not unlit:
        node = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -400, y_pos)
        node.set_editor_property("r", roughness)
        mel.connect_material_property(
            node, "", unreal.MaterialProperty.MP_ROUGHNESS)
        y_pos += 200

    # -- Metallic --
    if metallic > 0.01 and not unlit:
        node = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -400, y_pos)
        node.set_editor_property("r", metallic)
        mel.connect_material_property(
            node, "", unreal.MaterialProperty.MP_METALLIC)
        y_pos += 200

    # -- Emissive --
    if emissive_color:
        node = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -400, y_pos)
        scaled = unreal.LinearColor(
            emissive_color.r * emissive_strength,
            emissive_color.g * emissive_strength,
            emissive_color.b * emissive_strength,
            1.0)
        node.set_editor_property("constant", scaled)
        mel.connect_material_property(
            node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        y_pos += 200

    # -- Two-Sided --
    if two_sided:
        mat.set_editor_property("two_sided", True)

    # -- Unlit Shading --
    if unlit:
        mat.set_editor_property("shading_model",
                                unreal.MaterialShadingModel.MSM_UNLIT)

    # Recompile & save
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    materials_cache[name] = mat
    print(f"    + {name}")
    return mat


# ============================================================
# HELPER: Spawn Static Mesh Actor
# ============================================================
def spawn_mesh(label, mesh_path, location,
               rotation=None, scale=None, material=None):
    """Spawn a StaticMeshActor with optional material and scale."""
    if rotation is None:
        rotation = unreal.Rotator(0, 0, 0)

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, location, rotation)
    if not actor:
        print(f"    WARNING: Failed to spawn {label}")
        return None

    actor.set_actor_label(label)

    sm = actor.get_component_by_class(unreal.StaticMeshComponent)
    if sm:
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if mesh:
            sm.set_static_mesh(mesh)
        if material:
            sm.set_material(0, material)
        sm.set_mobility(unreal.ComponentMobility.MOVABLE)

    if scale:
        actor.set_actor_scale3d(scale)

    return actor


def attach_mesh(parent, label, mesh_path,
                rel_loc, rel_scale=None, material=None):
    """Spawn a mesh, attach it to parent, set relative transform."""
    ploc = parent.get_actor_location()
    actor = spawn_mesh(label, mesh_path, ploc, material=material)
    if not actor:
        return None

    actor.attach_to_actor(
        parent, "",
        unreal.AttachmentRule.SNAP_TO_TARGET,
        unreal.AttachmentRule.SNAP_TO_TARGET,
        unreal.AttachmentRule.SNAP_TO_TARGET,
        False)

    actor.set_actor_relative_location(rel_loc, False, False)
    if rel_scale:
        actor.set_actor_relative_scale3d(rel_scale)

    return actor


# ============================================================
# PHASE 1: MATERIALS
# ============================================================
def build_materials():
    """Create all PBR materials for the level."""
    global HAS_MATERIAL_EDITING

    print("\n[Phase 1/7] Creating Materials...")

    # Check if MaterialEditingLibrary is available
    try:
        _ = unreal.MaterialEditingLibrary
        _ = unreal.MaterialFactoryNew
        HAS_MATERIAL_EDITING = True
        print("  MaterialEditingLibrary available.")
    except AttributeError:
        HAS_MATERIAL_EDITING = False
        print("  WARNING: MaterialEditingLibrary not available.")
        print("  Level will be built with default materials.")
        return

    # -- Earth & Launch Pad --
    create_material("M_Ground",
        base_color=unreal.LinearColor(0.10, 0.14, 0.07, 1.0),
        roughness=0.90, metallic=0.0)

    create_material("M_LaunchPad",
        base_color=unreal.LinearColor(0.40, 0.38, 0.34, 1.0),
        roughness=0.92, metallic=0.0)

    create_material("M_LaunchStructure",
        base_color=unreal.LinearColor(0.18, 0.18, 0.20, 1.0),
        roughness=0.40, metallic=0.78)

    # -- Rocket --
    create_material("M_RocketBody",
        base_color=unreal.LinearColor(0.92, 0.93, 0.95, 1.0),
        roughness=0.12, metallic=0.85)

    create_material("M_RocketAccent",
        base_color=unreal.LinearColor(0.78, 0.08, 0.05, 1.0),
        roughness=0.30, metallic=0.35)

    create_material("M_CockpitFrame",
        base_color=unreal.LinearColor(0.03, 0.03, 0.05, 1.0),
        roughness=0.30, metallic=0.70)

    create_material("M_CockpitPanel",
        base_color=unreal.LinearColor(0.06, 0.07, 0.08, 1.0),
        roughness=0.50, metallic=0.40,
        emissive_color=unreal.LinearColor(0.0, 0.15, 0.08, 1.0),
        emissive_strength=0.3)

    # -- Moon --
    create_material("M_Moon",
        base_color=unreal.LinearColor(0.52, 0.50, 0.47, 1.0),
        roughness=0.96, metallic=0.0,
        emissive_color=unreal.LinearColor(0.06, 0.06, 0.055, 1.0),
        emissive_strength=1.0)

    # -- Space --
    create_material("M_SpaceSky",
        unlit=True, two_sided=True,
        emissive_color=unreal.LinearColor(0.002, 0.003, 0.010, 1.0),
        emissive_strength=1.0)

    create_material("M_Star",
        unlit=True,
        emissive_color=unreal.LinearColor(1.0, 0.97, 0.90, 1.0),
        emissive_strength=10.0)

    create_material("M_StarBlue",
        unlit=True,
        emissive_color=unreal.LinearColor(0.65, 0.80, 1.0, 1.0),
        emissive_strength=12.0)

    create_material("M_StarWarm",
        unlit=True,
        emissive_color=unreal.LinearColor(1.0, 0.55, 0.25, 1.0),
        emissive_strength=6.0)

    create_material("M_StarBright",
        unlit=True,
        emissive_color=unreal.LinearColor(1.0, 1.0, 1.0, 1.0),
        emissive_strength=25.0)

    print(f"  Created {len(materials_cache)} materials.")


# ============================================================
# PHASE 2: LIGHTING & ATMOSPHERE
# ============================================================
def build_lighting():
    """Set up the complete lighting and atmospheric system."""
    print("\n[Phase 2/7] Setting up Lighting & Atmosphere...")

    # -- Directional Light (Sun) --
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 100000),
        unreal.Rotator(-30, -45, 0))
    if sun:
        sun.set_actor_label("Sun_DirectionalLight")
        c = sun.get_component_by_class(unreal.DirectionalLightComponent)
        if c:
            c.set_editor_property("intensity", 10.0)
            c.set_editor_property("atmosphere_sun_light", True)
            c.set_editor_property("cast_cloud_shadows", True)
            c.set_editor_property("light_color",
                                  unreal.Color(255, 248, 230, 255))
            c.set_mobility(unreal.ComponentMobility.MOVABLE)
        print("    + Sun (DirectionalLight, Intensity=10)")

    # -- Sky Atmosphere --
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0))
    if sky:
        sky.set_actor_label("Earth_SkyAtmosphere")
        print("    + SkyAtmosphere")

    # -- Sky Light --
    skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 20000),
        unreal.Rotator(0, 0, 0))
    if skylight:
        skylight.set_actor_label("Space_SkyLight")
        sl = skylight.get_component_by_class(unreal.SkyLightComponent)
        if sl:
            sl.set_editor_property("real_time_capture", True)
            sl.set_editor_property("intensity", 1.5)
            sl.set_mobility(unreal.ComponentMobility.MOVABLE)
        print("    + SkyLight (RealTimeCapture)")

    # -- Volumetric Clouds --
    clouds = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.VolumetricCloud,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0))
    if clouds:
        clouds.set_actor_label("Earth_VolumetricClouds")
        print("    + VolumetricClouds")

    # -- Exponential Height Fog --
    fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0))
    if fog:
        fog.set_actor_label("Earth_HeightFog")
        fc = fog.get_component_by_class(
            unreal.ExponentialHeightFogComponent)
        if fc:
            try:
                fc.set_editor_property("fog_density", 0.004)
            except Exception:
                pass
            try:
                fc.set_editor_property("fog_height_falloff", 0.15)
            except Exception:
                pass
            try:
                fc.set_editor_property("start_distance", 800.0)
            except Exception:
                pass
            try:
                fc.set_editor_property("fog_inscattering_color",
                    unreal.LinearColor(0.45, 0.55, 0.75, 1.0))
            except Exception:
                try:
                    fc.set_editor_property("inscattering_color_cubemap_angle",
                        0.0)
                except Exception:
                    pass
            try:
                fc.set_editor_property("volumetric_fog", True)
            except Exception:
                pass
        print("    + ExponentialHeightFog (Density=0.004)")

    # -- Post Process Volume --
    ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0))
    if ppv:
        ppv.set_actor_label("Space_PostProcess")
        ppv.set_editor_property("unbound", True)
        s = ppv.get_editor_property("settings")

        # Apply each setting individually with error handling
        pp_settings = {
            "auto_exposure_min_brightness": 0.001,
            "auto_exposure_max_brightness": 20.0,
            "bloom_intensity": 2.2,
            "bloom_threshold": 0.4,
            "motion_blur_amount": 0.35,
            "vignette_intensity": 0.45,
            "scene_fringe_intensity": 0.25,
        }
        applied = []
        for prop, val in pp_settings.items():
            try:
                s.set_editor_property(prop, val)
                applied.append(prop)
            except Exception:
                pass

        ppv.set_editor_property("settings", s)
        print(f"    + PostProcessVolume ({len(applied)} settings applied)")


# ============================================================
# PHASE 3: EARTH SURFACE & LAUNCH PAD
# ============================================================
def build_ground():
    """Create the ground terrain and launch pad structures."""
    print("\n[Phase 3/7] Building Earth Surface & Launch Pad...")

    m_ground = materials_cache.get("M_Ground")
    m_pad = materials_cache.get("M_LaunchPad")
    m_struct = materials_cache.get("M_LaunchStructure")

    # -- Large Ground Plane --
    spawn_mesh("Earth_Ground",
        "/Engine/BasicShapes/Plane",
        unreal.Vector(0, 0, -30),
        scale=unreal.Vector(10000.0, 10000.0, 1.0),
        material=m_ground)
    print("    + Ground (1 km x 1 km)")

    # -- Launch Pad Platform --
    spawn_mesh("LaunchPad_Platform",
        "/Engine/BasicShapes/Cube",
        unreal.Vector(0, 0, 20),
        scale=unreal.Vector(30.0, 30.0, 0.4),
        material=m_pad)
    print("    + Launch Pad Platform")

    # -- Service Tower --
    spawn_mesh("LaunchPad_Tower",
        "/Engine/BasicShapes/Cube",
        unreal.Vector(-1400, 0, 2500),
        scale=unreal.Vector(3.5, 3.5, 50.0),
        material=m_struct)

    # -- Service Arm --
    spawn_mesh("LaunchPad_ServiceArm",
        "/Engine/BasicShapes/Cube",
        unreal.Vector(-700, 0, 3000),
        scale=unreal.Vector(14.0, 1.5, 0.6),
        material=m_struct)

    # -- Lightning Tower --
    spawn_mesh("LaunchPad_LightningTower",
        "/Engine/BasicShapes/Cylinder",
        unreal.Vector(-1400, 0, 5500),
        scale=unreal.Vector(0.5, 0.5, 12.0),
        material=m_struct)
    print("    + Service Tower + Arm + Lightning Tower")

    # -- Fuel Tanks --
    for i, (fx, fy) in enumerate([(1500, -700), (1500, 700)]):
        spawn_mesh(f"LaunchPad_FuelTank_{i+1}",
            "/Engine/BasicShapes/Cylinder",
            unreal.Vector(fx, fy, 500),
            scale=unreal.Vector(4.5, 4.5, 10.0),
            material=m_struct)
    print("    + 2 Fuel Tanks")

    # -- Ground Service Building --
    spawn_mesh("LaunchPad_ServiceBuilding",
        "/Engine/BasicShapes/Cube",
        unreal.Vector(3000, 0, 350),
        scale=unreal.Vector(12.0, 20.0, 7.0),
        material=m_pad)
    print("    + Service Building")

    # -- Pad Lights --
    light_positions = [
        (-1400, 600, 5200),
        (-1400, -600, 5200),
        (800, 800, 800),
        (800, -800, 800),
    ]
    for i, (lx, ly, lz) in enumerate(light_positions):
        light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.PointLight,
            unreal.Vector(lx, ly, lz),
            unreal.Rotator(0, 0, 0))
        if light:
            light.set_actor_label(f"Pad_Light_{i+1}")
            pl = light.get_component_by_class(
                unreal.PointLightComponent)
            if pl:
                pl.set_editor_property("intensity", 80000.0)
                pl.set_editor_property("light_color",
                    unreal.Color(255, 215, 170, 255))
                pl.set_editor_property("attenuation_radius", 10000.0)
                pl.set_mobility(unreal.ComponentMobility.MOVABLE)
    print("    + 4 Pad Lights")


# ============================================================
# PHASE 4: SPACE ENVIRONMENT
# ============================================================
def build_space():
    """Create the deep space background with stars."""
    print("\n[Phase 4/7] Building Space Environment...")

    m_sky = materials_cache.get("M_SpaceSky")
    star_mats = [
        materials_cache.get("M_Star"),
        materials_cache.get("M_Star"),
        materials_cache.get("M_Star"),
        materials_cache.get("M_StarBlue"),
        materials_cache.get("M_StarWarm"),
        materials_cache.get("M_StarBright"),
    ]
    # Filter out None values
    star_mats = [m for m in star_mats if m is not None]

    # -- Starfield Background Sphere --
    # Two-sided material ensures visibility from inside
    spawn_mesh("Starfield_Sphere",
        "/Engine/BasicShapes/Sphere",
        unreal.Vector(0, 0, MOON_ALTITUDE * 0.4),
        scale=unreal.Vector(STARFIELD_RADIUS,
                            STARFIELD_RADIUS,
                            STARFIELD_RADIUS),
        material=m_sky)
    print(f"    + Starfield Sphere (radius={STARFIELD_RADIUS/100:.0f}m)")

    # -- Individual Stars --
    placed = 0
    for i in range(STAR_COUNT):
        # Random direction on upper hemisphere
        theta = random.uniform(0, 2.0 * math.pi)
        # Bias toward upper hemisphere
        phi = random.uniform(0.05, math.pi * 0.85)
        dist = random.uniform(STAR_MIN_DIST, STAR_MAX_DIST)

        x = dist * math.sin(phi) * math.cos(theta)
        y = dist * math.sin(phi) * math.sin(theta)
        z = dist * math.cos(phi)

        # Ensure above ground
        z = max(z, 300000.0)

        # Star size — visible enough to create glow via bloom
        star_scale = random.uniform(100.0, 500.0)

        mat = random.choice(star_mats) if star_mats else None

        actor = spawn_mesh(f"Star_{i+1:02d}",
            "/Engine/BasicShapes/Sphere",
            unreal.Vector(x, y, z),
            scale=unreal.Vector(star_scale, star_scale, star_scale),
            material=mat)
        if actor:
            placed += 1

    print(f"    + {placed} Stars (emissive, varied colors)")

    # -- Milky Way Band (elongated glow strip) --
    m_star_bright = materials_cache.get("M_StarBright")
    if m_star_bright:
        # Create a stretched sphere as a diffuse band
        spawn_mesh("MilkyWay_Band",
            "/Engine/BasicShapes/Sphere",
            unreal.Vector(2000000, 0, 4000000),
            scale=unreal.Vector(80000.0, 8000.0, 3000.0),
            material=materials_cache.get("M_StarBlue"))
        print("    + Milky Way Band")


# ============================================================
# PHASE 5: MOON
# ============================================================
def build_moon():
    """Create the Moon at correct altitude with surface material."""
    print("\n[Phase 5/7] Building Moon...")

    m_moon = materials_cache.get("M_Moon")

    moon = spawn_mesh("Moon_Body",
        "/Engine/BasicShapes/Sphere",
        unreal.Vector(0, 0, MOON_ALTITUDE),
        scale=unreal.Vector(MOON_SCALE, MOON_SCALE, MOON_SCALE),
        material=m_moon)

    if moon:
        alt_km = MOON_ALTITUDE / 100000.0
        dia_km = MOON_SCALE * 100.0 / 100000.0
        print(f"    + Moon (altitude={alt_km:.0f}km, diameter={dia_km:.1f}km)")
        print(f"      Position: directly above (0, 0, {MOON_ALTITUDE:.0f})")


# ============================================================
# PHASE 6: PLAYER SPACESHIP
# ============================================================
def build_player_rocket():
    """Create the player pawn with rocket body and cockpit."""
    print("\n[Phase 6/7] Building Player Spaceship...")

    m_body = materials_cache.get("M_RocketBody")
    m_accent = materials_cache.get("M_RocketAccent")
    m_cockpit = materials_cache.get("M_CockpitFrame")
    m_panel = materials_cache.get("M_CockpitPanel")
    m_nozzle = materials_cache.get("M_LaunchStructure")

    # -- Player Pawn --
    pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DefaultPawn,
        unreal.Vector(0, 0, PLAYER_START_Z),
        unreal.Rotator(0, 0, 0))
    if not pawn:
        print("    ERROR: Failed to spawn player pawn!")
        return

    pawn.set_actor_label("Player_Spaceship")
    pawn.set_editor_property("auto_possess_player",
                             unreal.AutoReceiveInput.PLAYER0)

    # Movement speed
    mc = pawn.get_component_by_class(unreal.PawnMovementComponent)
    if mc:
        mc.set_editor_property("max_speed", PAWN_MAX_SPEED)
        mc.set_editor_property("acceleration", 60000.0)
        mc.set_editor_property("deceleration", 25000.0)
    print(f"    + Player Pawn (MaxSpeed={PAWN_MAX_SPEED/100:.0f} m/s)")

    # ---- COCKPIT WINDOW FRAME ----
    # These thin bars frame the player's view like a spacecraft canopy

    # Top frame bar
    attach_mesh(pawn, "Cockpit_Top",
        "/Engine/BasicShapes/Cube",
        rel_loc=unreal.Vector(220.0, 0.0, 85.0),
        rel_scale=unreal.Vector(0.25, 2.8, 0.03),
        material=m_cockpit)

    # Bottom frame bar
    attach_mesh(pawn, "Cockpit_Bottom",
        "/Engine/BasicShapes/Cube",
        rel_loc=unreal.Vector(220.0, 0.0, -58.0),
        rel_scale=unreal.Vector(0.25, 2.8, 0.03),
        material=m_cockpit)

    # Left frame bar
    attach_mesh(pawn, "Cockpit_Left",
        "/Engine/BasicShapes/Cube",
        rel_loc=unreal.Vector(220.0, -120.0, 14.0),
        rel_scale=unreal.Vector(0.25, 0.03, 1.5),
        material=m_cockpit)

    # Right frame bar
    attach_mesh(pawn, "Cockpit_Right",
        "/Engine/BasicShapes/Cube",
        rel_loc=unreal.Vector(220.0, 120.0, 14.0),
        rel_scale=unreal.Vector(0.25, 0.03, 1.5),
        material=m_cockpit)

    # Center vertical strut
    attach_mesh(pawn, "Cockpit_CenterStrut",
        "/Engine/BasicShapes/Cube",
        rel_loc=unreal.Vector(230.0, 0.0, 14.0),
        rel_scale=unreal.Vector(0.20, 0.02, 1.45),
        material=m_cockpit)

    print("    + Cockpit Window Frame (5 struts)")

    # ---- DASHBOARD / INSTRUMENT PANEL ----
    attach_mesh(pawn, "Cockpit_Dashboard",
        "/Engine/BasicShapes/Cube",
        rel_loc=unreal.Vector(170.0, 0.0, -45.0),
        rel_scale=unreal.Vector(1.0, 2.4, 0.03),
        material=m_panel)

    # Small instrument displays on dashboard
    for idx, dy in enumerate([-50, 0, 50]):
        attach_mesh(pawn, f"Cockpit_Instrument_{idx+1}",
            "/Engine/BasicShapes/Cube",
            rel_loc=unreal.Vector(168.0, float(dy), -38.0),
            rel_scale=unreal.Vector(0.12, 0.20, 0.01),
            material=m_panel)

    print("    + Dashboard + 3 Instrument Panels")

    # ---- ROCKET BODY ----
    # Main fuselage cylinder (below player)
    attach_mesh(pawn, "Rocket_Body",
        "/Engine/BasicShapes/Cylinder",
        rel_loc=unreal.Vector(0.0, 0.0, -450.0),
        rel_scale=unreal.Vector(2.2, 2.2, 9.0),
        material=m_body)

    # Accent stripe on body
    attach_mesh(pawn, "Rocket_Stripe",
        "/Engine/BasicShapes/Cylinder",
        rel_loc=unreal.Vector(0.0, 0.0, -200.0),
        rel_scale=unreal.Vector(2.25, 2.25, 0.5),
        material=m_accent)

    print("    + Rocket Body + Accent Stripe")

    # ---- NOSE CONE ----
    attach_mesh(pawn, "Rocket_NoseCone",
        "/Engine/BasicShapes/Sphere",
        rel_loc=unreal.Vector(0.0, 0.0, 550.0),
        rel_scale=unreal.Vector(2.2, 2.2, 3.5),
        material=m_body)
    print("    + Nose Cone")

    # ---- ENGINE NOZZLE ----
    attach_mesh(pawn, "Rocket_Nozzle",
        "/Engine/BasicShapes/Cylinder",
        rel_loc=unreal.Vector(0.0, 0.0, -1400.0),
        rel_scale=unreal.Vector(2.8, 2.8, 1.8),
        material=m_nozzle)

    # Inner nozzle glow
    m_star = materials_cache.get("M_StarWarm")
    attach_mesh(pawn, "Rocket_NozzleGlow",
        "/Engine/BasicShapes/Sphere",
        rel_loc=unreal.Vector(0.0, 0.0, -1500.0),
        rel_scale=unreal.Vector(1.8, 1.8, 0.8),
        material=m_star)
    print("    + Engine Nozzle + Glow")

    # ---- FINS (4x at 90-degree intervals) ----
    fin_offsets = [
        (250.0, 0.0),    # +X
        (-250.0, 0.0),   # -X
        (0.0, 250.0),    # +Y
        (0.0, -250.0),   # -Y
    ]
    for i, (fx, fy) in enumerate(fin_offsets):
        # Determine fin orientation
        if abs(fx) > abs(fy):
            fin_scale = unreal.Vector(0.12, 1.5, 2.5)
        else:
            fin_scale = unreal.Vector(1.5, 0.12, 2.5)

        attach_mesh(pawn, f"Rocket_Fin_{i+1}",
            "/Engine/BasicShapes/Cube",
            rel_loc=unreal.Vector(fx, fy, -1100.0),
            rel_scale=fin_scale,
            material=m_body)
    print("    + 4 Rocket Fins")


# ============================================================
# PHASE 7: BLUEPRINT ARCHITECTURE
# ============================================================
def create_blueprints():
    """Create GameMode and HUD Blueprint assets."""
    print("\n[Phase 7/7] Creating Blueprint Architecture...")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # -- BP_SpaceHUD --
    hud_asset = f"{BLUEPRINTS_PATH}/BP_SpaceHUD"
    if not unreal.EditorAssetLibrary.does_asset_exist(hud_asset):
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.HUD)
        hud_bp = asset_tools.create_asset(
            "BP_SpaceHUD", BLUEPRINTS_PATH, None, factory)
        if hud_bp:
            # Add variables for HUD display
            try:
                pin_bool = unreal.EdGraphPinType()
                pin_bool.set_editor_property("pin_category", "bool")
                unreal.BlueprintEditorLibrary.add_member_variable(
                    hud_bp, "bShowHints", pin_bool)
            except Exception:
                pass
            unreal.EditorAssetLibrary.save_loaded_asset(hud_bp)
            print("    + BP_SpaceHUD (HUD subclass)")
    else:
        print("    = BP_SpaceHUD already exists")

    # -- BP_SpaceGameMode --
    gm_asset = f"{BLUEPRINTS_PATH}/BP_SpaceGameMode"
    if not unreal.EditorAssetLibrary.does_asset_exist(gm_asset):
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class",
                                    unreal.GameModeBase)
        gm_bp = asset_tools.create_asset(
            "BP_SpaceGameMode", BLUEPRINTS_PATH, None, factory)
        if gm_bp:
            # Set HUD class on the GameMode CDO
            try:
                hud_class = unreal.EditorAssetLibrary.load_blueprint_class(
                    hud_asset)
                gm_class = unreal.EditorAssetLibrary.load_blueprint_class(
                    gm_asset)
                if hud_class and gm_class:
                    cdo = unreal.get_default_object(gm_class)
                    cdo.set_editor_property("hud_class", hud_class)
                    print("    -> HUD class linked to GameMode")
            except Exception as e:
                print(f"    Note: Auto-link HUD failed ({e})")
                print("    -> Set manually: BP_SpaceGameMode > HUD Class > BP_SpaceHUD")

            unreal.EditorAssetLibrary.save_loaded_asset(gm_bp)
            print("    + BP_SpaceGameMode (GameModeBase subclass)")
    else:
        print("    = BP_SpaceGameMode already exists")


# ============================================================
# MAIN EXECUTION
# ============================================================
def main():
    print("\nInitializing...")

    # Load or create map
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    print(f"Map loaded: {MAP_PATH}")

    # Clear all existing actors
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    count = 0
    for a in subsystem.get_all_level_actors():
        subsystem.destroy_actor(a)
        count += 1
    print(f"Cleared {count} existing actors.")

    # Build each phase
    build_materials()
    build_lighting()
    build_ground()
    build_space()
    build_moon()
    build_player_rocket()

    try:
        create_blueprints()
    except Exception as e:
        print(f"\nBlueprint creation skipped: {e}")

    # Save the map
    print("\nSaving map...")
    unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)

    # Summary
    print("\n" + "=" * 70)
    print("  SPACE ODYSSEY — BUILD COMPLETE!")
    print("=" * 70)
    print(f"  Map:       {MAP_PATH}")
    print(f"  Materials: {len(materials_cache)} created")
    print(f"  Moon:      altitude {MOON_ALTITUDE/100000:.0f} km, "
          f"diameter {MOON_SCALE * 0.001:.1f} km")
    print(f"  Stars:     {STAR_COUNT} individual stars + background sphere")
    print(f"  Speed:     {PAWN_MAX_SPEED/100:.0f} m/s (Mach 3)")
    print("=" * 70)
    print("\n  Next: Run Play_SpaceFlight.bat to play!")
    print("  Controls: W/S=Forward/Back, Space=Up, Mouse=Look")

main()
