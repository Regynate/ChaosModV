#pragma once

namespace Hooks
{
	void RouteKey(int ogAction, int routeAction);
	void RouteKeyWithTransform(int ogAction, int routeAction, std::function<float(float)> transform);
	void ResetKeyRoute(int ogAction);
	void ResetKeyRoutes();
	void SetInputDelayMs(int delayMs);
}