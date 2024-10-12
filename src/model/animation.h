#ifndef ANIMATION_H
#define ANIMATION_H

class Animation {
public:
	bool running = false;
	bool finished = true;
	float animationStartTime = 0.0f;
	float duration = 0.0f;
	float animationStep = 0.1f;

	void updateAnimation(float currentTime) {
		if (!running)
			return;

		animationStep = (currentTime - animationStartTime) / duration;
		animationStep = sqrtf(animationStep);
		if (animationStep > 1.0f) {
			running = false;
			finished = true;
		}
	}

	void doAnimation(float animationDuration, float startTime) {
		if (running)
			return;

		animationStartTime = startTime;
		duration = animationDuration;
		running = true;
		finished = false;
		animationStep = 0.0f;
	}
};

#endif