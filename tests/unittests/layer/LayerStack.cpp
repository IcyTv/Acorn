#include "gtest/gtest.h"
#include <Acorn/layer/Layer.h>
#include <Acorn/layer/LayerStack.h>

#include <memory>

class TestLayer : public Acorn::Layer
{
public:
	TestLayer()
		: Layer("TestLayer")
	{
	}

	TestLayer(int id)
		: Layer("TestLayer"), Id(id)
	{
	}

	virtual void OnAttach() override
	{
		Attached = true;
	}

	int Id = 0;
	bool Attached = false;
};

TEST(LayerStack, Layer)
{
	Acorn::LayerStack layerStack;
	TestLayer layer;
	layerStack.PushLayer(&layer);
	EXPECT_EQ(layerStack.GetLayerCount(), 1) << "Layer should be in the layer stack";
	EXPECT_TRUE(layer.Attached) << "Layer should be attached";
	layerStack.PopLayer(&layer);
	EXPECT_EQ(layerStack.GetLayerCount(), 0) << "Layer should be removed from the layer stack";
}

TEST(LayerStack, Overlay)
{
	Acorn::LayerStack layerStack;
	TestLayer layer;
	layerStack.PushOverlay(&layer);
	EXPECT_EQ(layerStack.GetLayerCount(), 1) << "Overlay should be in the layer stack";
	EXPECT_TRUE(layer.Attached) << "Overlay should be attached";
	layerStack.PopOverlay(&layer);
	EXPECT_EQ(layerStack.GetLayerCount(), 0) << "Overlay should be removed from the layer stack";
}

TEST(LayerStack, LayerAndOverlay)
{
	Acorn::LayerStack layerStack;
	TestLayer layer;
	TestLayer overlay{1};
	layerStack.PushLayer(&layer);
	layerStack.PushOverlay(&overlay);
	EXPECT_EQ(layerStack.GetLayerCount(), 2) << "Layer and Overlay should be in the LayerStack";

	int id = 0;
	for (auto& layer : layerStack)
	{
		EXPECT_STREQ(layer->GetName().c_str(), "TestLayer") << "Layer name is not 'TestLayer'";
		TestLayer* testLayer = static_cast<TestLayer*>(layer);
		EXPECT_EQ(testLayer->Id, id++) << "Layer id should be 0 or 1";
		EXPECT_TRUE(testLayer->Attached) << "Layer " << testLayer->Id << " was not attached";
	}

	layerStack.PopOverlay(&overlay);
	EXPECT_EQ(layerStack.GetLayerCount(), 1) << "PopOverlay should remove the overlay from the layer stack";
	layerStack.PopLayer(&layer);
	EXPECT_EQ(layerStack.GetLayerCount(), 0) << "PopLayer should remove the layer from the layer stack";
}