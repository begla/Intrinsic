function tick(p_InstanceId, p_DeltaT)
  local treeEntity = entity.getEntityByName(Name.new("RotatingTree"))
  local treeNode = nodeComponent.getComponentForEntity(treeEntity)

  local smallRotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(treeNode);
  nodeComponent.setOrientation(treeNode, glm.rotate(smallRotation, orientation))

  nodeComponent.updateTransforms(treeNode)
end

function onCreate(p_InstanceId, p_DeltaT)
end

function onDestroy(p_InstanceId, p_DeltaT)
end
