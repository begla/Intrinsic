function schatzi(p_DeltaT)
  data.timePassed = data.timePassed + p_DeltaT

  if data.timePassed > 5.0 then
   print("5 Sekunden vorbei")
   data.timePassed = 0.0
  end
end

function createPlayer()
  newPlayerEntity = entity.createEntity(Name.new("Player0"))
  
  newPlayerNode = nodeComponent.createNode(newPlayerEntity) 
  nodeComponent.attachChild(world.getRootNode(), newPlayerNode)
  nodeComponent.rebuildTreeAndUpdateTransforms()
  
  newPlayerMesh = meshComponent.createMesh(newPlayerEntity)
  meshComponent.setMeshName(newPlayerMesh, Name.new("PalmTree"))
  meshComponent.createResources(newPlayerMesh)

  return newPlayerNode
end

function tick(p_InstanceId, p_DeltaT)
  schatzi(p_InstanceId, p_DeltaT)
end

function onCreate(p_InstanceId, p_DeltaT)
  if not data then
    data = {}
  end

  data.playerNode = createPlayer()
  data.timePassed = 0.0 
end

function onDestroy(p_InstanceId, p_DeltaT)
end
