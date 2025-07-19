#ifndef MYCAMERA_H
#define MYCAMERA_H
/**	resources and credit for parts of the code in this source:
        Joey de Vries
        https://learnopengl.com/#!Getting-started/Camera
        https://joeydevries.com/#home

        Hammad Mazhar
        hammad (at) hamelot.io
        http://hamelot.io/visualization/moderngl-camera/

        Laurie Bradshaw
        see comment section from https://learnopengl.com/#!Getting-started/Camera

        resource and credit for how Quaternion words:
        A lot of great websites so additional credits goes to all the	authors out there who
        are contributing to the world wide collection of knoweldge.
**/



#include<glad/glad.h>	// for opengl function and type

// for matrices and vector calculation
#define GLM_FORCE_RADIANS
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

enum class TravelDirection{
        UP, DOWN, LEFT, RIGHT, FORWARD, BACKWARD
};

// FOV zooming
enum class ZoomState{
        _IN, _OUT
};

class Camera{
        private:
                float FOV;
                float zoom;
                float zoomDelta	= glm::radians(1.0f);

                float aspect;
                float nearClip;
                float farClip;

                glm::vec3 position;

                glm::quat orientation;					// store rotation and orientation data

                glm::vec3 mousePosition;				// use to calculate the yaw and pitch rotation.
                float mouseSenstivitiy	= 0.0009f;		// slow down the rate in which the camera rotate

                // play around with those two values until you get a roll speed that you like.
                const float ROLL_ANGLE	= 0.009f;		// we're using this because roll depends on the mouse scroll.
                float rollDamp			= 0.95f;		// we're using this because roll depends on the mouse scroll.

                float damp				= 0.8f;			// a value to damp the camera rotational speed

                // the angles to rotate the camera by.
                float yaw	= 0.0f;						// rotate about the y axis
                float pitch	= 0.0f;						// rotate about the x axis
                float roll	= 0.0f;						// rotate about the z axis
                float twoPI = glm::two_pi<float>();		// check radian bound

                float speed = 0.2f;						// the linear travel speed of the camera

                glm::mat4 perspectiveMatrix;
                //glm::mat4 viewMatrix;

                glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

                void rotate(float angle, const glm::vec3 &axis);

        public:
                Camera() = delete;

                ///<param name="FOV">The vertical Field of View. In radians</param>
                ///<param name="windowWidth">The width of the display window</param>
                ///<param name="windowHeight">The height of the display window</param>
                ///<param name="near">The near-clipping plane</param>
                ///<param name="far">The far-clipping plane</param>
                ///<param name="position">The position of the camera</param>
                Camera(float fov, int width, int height, float near, float far);
                Camera(float fov, int width, int height, float near, float far, glm::vec3 position);


                void Update(float delta = 1.0f);
                void Move(TravelDirection td);
                void MouseMoveEvent(float x, float y);		// control pitch and yaw
                void MouseScrollEvent(float yOffset);		// control roll
                void Zoom(ZoomState z);

                void SetAspect(float aspect);

                void SetMousePosition(float x, float y);
                void GetMatrices(glm::mat4& perspective, glm::mat4& view);
};


// ctor
Camera::Camera(float fov,  int width, int height, float nearClip, float farClip)
        : Camera(fov, width, height, nearClip, farClip, glm::vec3(0.0f, 0.0f, 1.0f))
{}
Camera::Camera(float fov, int width, int height, float nearClip, float farClip, glm::vec3 position)
        : FOV(fov), zoom(fov), aspect(float(width)/float(height)), nearClip(nearClip), farClip(farClip), position(position){

        perspectiveMatrix = glm::perspective(FOV, aspect, nearClip, farClip);
        mousePosition.x = width / 2.0f;
        mousePosition.y = height / 2.0f;
}

void Camera::Update(float delta){
        if(pitch != 0.0f)		rotate(pitch, xAxis);
        if(yaw != 0.0f)			rotate(yaw, yAxis);
        if(roll != 0.0f)		rotate(roll, zAxis);
        //
        //// instead of setting to zero immediately, we have the value eventually go to zero.
        pitch	*= damp;
        yaw		*= damp;
        roll	*= rollDamp;
}

void Camera::rotate(float angle, const glm::vec3 &axis){
        orientation *= glm::angleAxis(angle, axis * orientation);
}

void Camera::Move(TravelDirection td){

        // basically, translating the position vector

        glm::vec3 temp;
        switch(td){
                case TravelDirection::UP:
                        position -= glm::vec3(0.0f, speed, 0.0f) * orientation;
                        break;
                case TravelDirection::DOWN:
                        position += glm::vec3(0.0f, speed, 0.0f) * orientation;
                        break;
                case TravelDirection::LEFT:
                        position += glm::vec3(speed, 0.0f, 0.0f) * orientation;
                        break;
                case TravelDirection::RIGHT:
                        position -= glm::vec3(speed, 0.0f, 0.0f) * orientation;
                        break;
                case TravelDirection::FORWARD:
                        position += glm::vec3(0.0f, 0.0f, speed) * orientation;
                        break;
                case TravelDirection::BACKWARD:
                        position -= glm::vec3(0.0f, 0.0f, speed) * orientation;
                        break;
        }
}


void Camera::MouseMoveEvent(float x, float y){
        // updateing yaw
        yaw += (x - mousePosition.x) * mouseSenstivitiy;
        if(yaw > twoPI) yaw -= twoPI;
        else if(yaw < -twoPI) yaw += twoPI;

        // updateing pitch
        pitch += (y - mousePosition.y) * mouseSenstivitiy;
        if(pitch > twoPI) pitch -= twoPI;
        else if(pitch < -twoPI) pitch += twoPI;

        mousePosition.x = x;
        mousePosition.y = y;
}

void Camera::MouseScrollEvent(float yOffset){
        roll -= yOffset * ROLL_ANGLE;
        if(roll > twoPI) roll -= twoPI;
        else if(roll < -twoPI) roll += twoPI;
}

void Camera::Zoom(ZoomState z){
        bool recalculatePerspective = true;
        switch(z){
                case ZoomState::_IN:
                        if((zoom -= zoomDelta) < 0){
                                zoom = 0;
                                recalculatePerspective = false;
                        }
                        break;
                case ZoomState::_OUT:
                        if((zoom += zoomDelta) > FOV){
                                zoom = FOV;
                                recalculatePerspective = false;
                        }
                        break;
        }
        if(recalculatePerspective) perspectiveMatrix = glm::perspective(zoom, aspect, nearClip, farClip);
}

void Camera::SetAspect(float aspect){
        this->aspect = aspect;
        perspectiveMatrix = glm::perspective(zoom, aspect, nearClip, farClip);
}


// ideally you want to set the mouse position to the center of the scene
void Camera::SetMousePosition(float x, float y){
        mousePosition.x = x;
        mousePosition.y = y;
}

void Camera::GetMatrices(glm::mat4& perspective, glm::mat4& view){
        perspective = perspectiveMatrix;
        view = glm::translate(glm::mat4_cast(orientation), position);;
}




#endif // MYCAMERA_H
