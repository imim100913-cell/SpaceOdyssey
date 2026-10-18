import unreal

print("=== Setting up fully interactive UE5 Space Flight Map ===")

# 1. Load the map
map_path = "/Game/Maps/SpaceFlight_Cinematic"
world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)

# 2. Clear previous actors and rebuild cleanly
editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = editor_actor_subsystem.get_all_level_actors()
for a in all_actors:
    editor_actor_subsystem.destroy_actor(a)

print("Cleared old actors.")

# 3. Spawn Sun (Directional Light)
sun = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 50000), unreal.Rotator(-40, -30, 0))
if sun:
    sun.set_actor_label("Sun_DirectionalLight")
    comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if comp:
        comp.set_editor_property("intensity", 15.0)
        comp.set_editor_property("atmosphere_sun_light", True)
        comp.set_editor_property("cast_cloud_shadows", True)
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)

# 4. Spawn Sky Atmosphere (Earth Atmosphere)
sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
if sky:
    sky.set_actor_label("Earth_SkyAtmosphere")

# 5. Spawn Sky Light (Realtime ambient)
skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 10000), unreal.Rotator(0, 0, 0))
if skylight:
    skylight.set_actor_label("Space_SkyLight")
    sl_comp = skylight.get_component_by_class(unreal.SkyLightComponent)
    if sl_comp:
        sl_comp.set_editor_property("real_time_capture", True)
        sl_comp.set_mobility(unreal.ComponentMobility.MOVABLE)

# 6. Spawn Volumetric Clouds
clouds = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.VolumetricCloud, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
if clouds:
    clouds.set_actor_label("Earth_Clouds")

# 7. Spawn PostProcessVolume (Space Exposure & Bloom)
ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
if ppv:
    ppv.set_actor_label("Space_PostProcess")
    ppv.set_editor_property("unbound", True)
    s = ppv.get_editor_property("settings")
    s.set_editor_property("auto_exposure_min_brightness", 0.0)
    s.set_editor_property("auto_exposure_max_brightness", 15.0)
    s.set_editor_property("bloom_intensity", 1.5)
    s.set_editor_property("motion_blur_amount", 0.4)
    ppv.set_editor_property("settings", s)

# 8. Spawn Earth Surface/Ground Mesh
earth_ground = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -100), unreal.Rotator(0, 0, 0))
if earth_ground:
    earth_ground.set_actor_label("Earth_Surface")
    earth_ground.set_actor_scale3d(unreal.Vector(5000.0, 5000.0, 1.0))
    sm_comp = earth_ground.get_component_by_class(unreal.StaticMeshComponent)
    if sm_comp:
        plane_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane")
        if plane_mesh:
            sm_comp.set_static_mesh(plane_mesh)

# 9. Spawn Spaceship Pawn (Flying Pawn with Camera & Auto-possess)
pawn_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DefaultPawn, unreal.Vector(0, 0, 1500), unreal.Rotator(0, 0, 0))
if pawn_actor:
    pawn_actor.set_actor_label("Player_Spaceship")
    pawn_actor.set_editor_property("auto_possess_player", unreal.AutoReceiveInput.PLAYER0)
    move_comp = pawn_actor.get_component_by_class(unreal.PawnMovementComponent)
    if move_comp:
        move_comp.set_editor_property("max_speed", 15000.0)
        move_comp.set_editor_property("acceleration", 8000.0)
        move_comp.set_editor_property("deceleration", 4000.0)

# 10. Spawn Moon Sphere in the Sky (Far Distance)
moon_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(500000.0, 300000.0, 800000.0), unreal.Rotator(0, 0, 0))
if moon_actor:
    moon_actor.set_actor_label("Moon_CelestialBody")
    moon_actor.set_actor_scale3d(unreal.Vector(500.0, 500.0, 500.0))
    m_sm_comp = moon_actor.get_component_by_class(unreal.StaticMeshComponent)
    if m_sm_comp:
        sphere_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Sphere")
        if sphere_mesh:
            m_sm_comp.set_static_mesh(sphere_mesh)

# 11. Save the map
unreal.EditorLoadingAndSavingUtils.save_map(world, map_path)
print("=== UE5 Space Flight Map Saved Successfully! ===")
