#pragma once

//For use by client applications

#include "Acorn/core/Application.h"
#include "Acorn/core/Log.h"
#include "Acorn/layer/Layer.h"

#include "Acorn/core/Timestep.h"

#include "Acorn/renderer/2d/Renderer2D.h"
#include "Acorn/renderer/Buffer.h"
#include "Acorn/renderer/DebugRenderer.h"
#include "Acorn/renderer/Framebuffer.h"
#include "Acorn/renderer/Renderer.h"
#include "Acorn/renderer/Shader.h"
#include "Acorn/renderer/Texture.h"
#include "Acorn/renderer/VertexArray.h"

#include "Acorn/renderer/Camera.h"

#include "Acorn/templates/OrthographicCameraController.h"
#include "Acorn/utils/PlatformUtils.h"

#include "Acorn/serialize/Serializer.h"

#include "Acorn/ecs/Entity.h"
#include "Acorn/ecs/Scene.h"
#include "Acorn/ecs/components/Components.h"
#include "Acorn/ecs/components/ScriptableEntity.h"

#include "Acorn/input/Input.h"
#include "Acorn/input/KeyCodes.h"
#include "Acorn/input/MouseButtonCodes.h"

#include "Acorn/math/Math.h"
