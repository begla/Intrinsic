function tick(p_EntityRef, p_DeltaT)
  local nodeRef = nodeComponent.getComponentForEntity(p_EntityRef)

  local rotation = Quat.new(Vec3.new(0.0, p_DeltaT * 0.5, 0.0))
  local orientation = nodeComponent.getOrientation(nodeRef);
  nodeComponent.setOrientation(nodeRef, glm.rotate(rotation, orientation))

  nodeComponent.updateTransforms(nodeRef)
end

function onCreate(p_EntityRef, p_DeltaT)
end

function onDestroy(p_EntityRef, p_DeltaT)
end
