treeMeshNames = { "PalmTree", "PlamTree02", "PalmeTree03", "PineTree", "Tree04", 
  "Tree05", "Tree06", "Tree07", "Tree08", "Tree09", "Tree10", "Tree11", "Tree12", "Tree13", "Tree14", "Tree15", "Tree16" }

function spawnTree(p_InstanceId)
  if #simpleScript.treeEntities[p_InstanceId] >= 150 then
    return
  end

  -- Create root node
  local rootNodeEntity = entity.getEntityByName(Name.new("ScriptTrees" .. p_InstanceId))
  local rootNode = nodeComponent.getComponentForEntity(rootNodeEntity)
  
  if not rootNode:isValid() then
    local newRootNodeEntity = entity.createEntity(Name.new("ScriptTrees" .. p_InstanceId))
    rootNode = nodeComponent.createNode(newRootNodeEntity)
    nodeComponent.attachChild(world.getRootNode(), rootNode)
  end

  local newTreeEntity = entity.createEntity(Name.new("_Tree" .. #simpleScript.treeEntities[p_InstanceId]))
  local newTreeNode = nodeComponent.createNode(newTreeEntity)
  nodeComponent.attachChild(rootNode, newTreeNode)

  local position = nodeComponent.getPosition(newTreeNode)
  position.x = math.random(-500, 500)
  position.z = math.random(-500, 500)

  local size = nodeComponent.getSize(newTreeNode)
  local randomSize = math.random(1, 10) / 5.0
  size.x = randomSize
  size.y = randomSize
  size.z = randomSize

  nodeComponent.rebuildTreeAndUpdateTransforms()

  local newTreeMesh = meshComponent.createMesh(newTreeEntity)
  meshComponent.setMeshName(newTreeMesh, Name.new(treeMeshNames[math.random(1, #treeMeshNames)]))
  meshComponent.createResources(newTreeMesh)

  table.insert(simpleScript.treeEntities[p_InstanceId], newTreeEntity)
end

function moveAndRotateTrees(p_InstanceId, p_DeltaT)
  local smallRotation = Quat.new(Vec3.new(0.0, p_DeltaT * 2.0, 0.0))

  for i=#simpleScript.treeEntities[p_InstanceId], 1, -1 do  
    local treeEntity = simpleScript.treeEntities[p_InstanceId][i]

    if not treeEntity:isValid() or not entity.isAlive(treeEntity) then
      table.remove(simpleScript.treeEntities[p_InstanceId], i)
    else
      local treeNode = nodeComponent.getComponentForEntity(treeEntity)

      local orientation = nodeComponent.getOrientation(treeNode);
      nodeComponent.setOrientation(treeNode, glm.rotate(smallRotation, orientation))

      local position = nodeComponent.getPosition(treeNode);
      position.y = math.abs(math.sin(position.x + position.z + simpleScript.timePassed[p_InstanceId] * 2.0)) * 0.1 * i    
    end
  end

  local rootNode = nodeComponent.getComponentForEntity(entity.getEntityByName(Name.new("ScriptTrees" .. p_InstanceId)))
  if not rootNode:isValid() then
    return
  end

  nodeComponent.updateTransforms(rootNode)
end

function spawnTrees(p_InstanceId, p_DeltaT) 
  if (simpleScript.timePassedSinceLastTree[p_InstanceId] > 0.001) then
    spawnTree(p_InstanceId)
    simpleScript.timePassedSinceLastTree[p_InstanceId] = 0.0
  end

  simpleScript.timePassedSinceLastTree[p_InstanceId] = simpleScript.timePassedSinceLastTree[p_InstanceId] + p_DeltaT
end

function moveCamera(p_InstanceId, p_DeltaT)
  local cameraEntity = entity.getEntityByName(Name.new("MainCamera"))
  if not cameraEntity:isValid() then
    return
  end

  local cameraNode = nodeComponent.getComponentForEntity(cameraEntity)
  local position = nodeComponent.getPosition(cameraNode)

  if inputSystem.keyState(0) == 1 then
    local smallRotation = Quat.new(Vec3.new(0.0, p_DeltaT, 0.0))
    local orientation = nodeComponent.getOrientation(cameraNode);
    nodeComponent.setOrientation(cameraNode, glm.rotate(smallRotation, orientation))
  end
  if inputSystem.keyState(1) == 1 then
    local smallRotation = Quat.new(Vec3.new(0.0, -p_DeltaT, 0.0))
    local orientation = nodeComponent.getOrientation(cameraNode);
    nodeComponent.setOrientation(cameraNode, glm.rotate(smallRotation, orientation))
  end
  if inputSystem.keyState(2) == 1 then
  	position.x = position.x - 12.0 * p_DeltaT
  end
  if inputSystem.keyState(3) == 1 then
  	position.x = position.x + 12.0 * p_DeltaT
  end

  nodeComponent.updateTransforms(cameraNode)
end

-- <-

function tick(p_InstanceId, p_DeltaT)
  moveCamera(p_InstanceId, p_DeltaT)
  spawnTrees(p_InstanceId, p_DeltaT)
  moveAndRotateTrees(p_InstanceId, p_DeltaT)
  simpleScript.timePassed[p_InstanceId] = simpleScript.timePassed[p_InstanceId] + p_DeltaT
end

function onCreate(p_InstanceId, p_DeltaT)
  if not simpleScript then
    simpleScript = {}
    simpleScript.timePassedSinceLastTree = {}
    simpleScript.timePassed = {}
    simpleScript.treeEntities = {}
  end

  simpleScript.timePassed[p_InstanceId] = 0.0
  simpleScript.treeEntities[p_InstanceId] = {}
  simpleScript.timePassedSinceLastTree[p_InstanceId] = 0.0

  print("Simple script instance #" .. p_InstanceId .. " created...")
end

function onDestroy(p_InstanceId, p_DeltaT)
  local rootNode = nodeComponent.getComponentForEntity(entity.getEntityByName(Name.new("ScriptTrees" .. p_InstanceId)))
  if rootNode:isValid() then
    world.destroyNodeFull(rootNode)
  end

  simpleScript.treeEntities[p_InstanceId] = {}
  simpleScript.timePassedSinceLastTree[p_InstanceId] = 0.0
  simpleScript.timePassed[p_InstanceId] = 0.0

  print("Simple script instance #" .. p_InstanceId .. " destroyed...")
end
