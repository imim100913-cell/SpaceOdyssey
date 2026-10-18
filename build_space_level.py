import unreal

print("=== Building Cinematic Space Flight Level in UE5 ===")

# 1. Create a new blank map
world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)

# 2. Spawn Directional Light (Sun)
sun_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 5000), unreal.Rotator(-45, -30, 0))
if sun_actor:
    sun_actor.set_actor_label("Sun_DirectionalLight")
    sun_comp = sun_actor.get_component_by_class(unreal.DirectionalLightComponent)
    if sun_comp:
        sun_comp.set_editor_property("intensity", 12.0)
        sun_comp.set_editor_property("atmosphere_sun_light", True)
        sun_comp.set_editor_property("cast_cloud_shadows", True)
        sun_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    print("Spawned DirectionalLight (Sun)")

# 3. Spawn SkyAtmosphere (Atmospheric scattering)
sky_atm = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
if sky_atm:
    sky_atm.set_actor_label("Atmosphere_Sky")
    print("Spawned SkyAtmosphere")

# 4. Spawn SkyLight (Real-time ambient lighting)
sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 500), unreal.Rotator(0, 0, 0))
if sky_light:
    sky_light.set_actor_label("Sky_Light_Ambient")
    sl_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
    if sl_comp:
        sl_comp.set_editor_property("real_time_capture", True)
        sl_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    print("Spawned SkyLight")

# 5. Spawn Volumetric Cloud (Dramatic Cloud Layers)
vol_cloud = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.VolumetricCloud, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
if vol_cloud:
    vol_cloud.set_actor_label("Volumetric_Clouds")
    print("Spawned VolumetricCloud")

# 6. Spawn PostProcessVolume (Auto-exposure & Space Bloom)
ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
if ppv:
    ppv.set_actor_label("Space_PostProcess")
    ppv.set_editor_property("unbound", True)
    settings = ppv.get_editor_property("settings")
    settings.set_editor_property("auto_exposure_min_brightness", 0.0)
    settings.set_editor_property("auto_exposure_max_brightness", 15.0)
    settings.set_editor_property("bloom_intensity", 1.5)
    settings.set_editor_property("motion_blur_amount", 0.4)
    ppv.set_editor_property("settings", settings)
    print("Spawned PostProcessVolume")

# 7. Spawn PlayerStart (Starting Position on Earth)
player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 3000), unreal.Rotator(0, 0, 0))
if player_start:
    player_start.set_actor_label("Spaceship_PlayerStart")
    print("Spawned PlayerStart at (0, 0, 3000)")

# 8. Save the map to /Game/Maps/SpaceFlight_Cinematic
save_path = "/Game/Maps/SpaceFlight_Cinematic"
success = unreal.EditorLoadingAndSavingUtils.save_map(world, save_path)
print(f"Map saved to {save_path}: Success = {success}")

print("=== Level Generation Complete! ===")
