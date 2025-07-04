#ifndef URDF_SAMPLES_H
#define URDF_SAMPLES_H

#define MSTRINGIFY(A) #A

tukk urdf_char2 = MSTRINGIFY(
	<robot name = "test_robot">
	<link name = "link1" />
	<link name = "link2" />
	<link name = "link3" />
	<link name = "link4" />

	<joint name = "joint1" type = "continuous">
	<parent link = "link1" />
	<child link = "link2" />
	</ joint>

	<joint name = "joint2" type = "continuous">
	<parent link = "link1" />
	<child link = "link3" />
	</ joint>

	<joint name = "joint3" type = "continuous">
	<parent link = "link3" />
	<child link = "link4" />
	</ joint>
	</ robot>);

tukk urdf_char1 = MSTRINGIFY(
                                   <?xml version="1.0"?>
                                   <robot name="myfirst">
                                   <link name="base_link">
                                   <visual>
                                   <geometry>
                                   <cylinder length="0.6" radius="0.2"/>
                                   </geometry>
                                   </visual>
                                   </link>
                                   </robot>
                                   );

tukk urdf_char3 = MSTRINGIFY(<?xml version="1.0"?>
                                   <robot name="multipleshapes">
                                   <link name="base_link">
                                   <visual>
                                   <geometry>
                                   <cylinder length="0.6" radius="0.2"/>
                                   </geometry>
                                   </visual>
                                   </link>
                                   
                                   <link name="right_leg">
                                   <visual>
                                   <geometry>
                                   <box size="0.6 .2 .1"/>
                                   </geometry>
                                   </visual>
                                   </link>
                                   
                                   <joint name="base_to_right_leg" type="fixed">
                                   <parent link="base_link"/>
                                   <child link="right_leg"/>
                                   </joint>
                                   
                                   </robot>);

tukk urdf_char4 = MSTRINGIFY(

								   <?xml version="1.0"?>
								   <robot name="materials">
								   <link name="base_link">
								   <visual>
								   <geometry>
								   <cylinder length="0.6" radius="0.2"/>
								   </geometry>
								   <material name="blue">
								   <color rgba="0 0 .8 1"/>
								   </material>
								   </visual>
								   </link>
								   
								   <link name="right_leg">
								   <visual>
								   <geometry>
								   <box size="0.6 .2 .1"/>
								   </geometry>
								   <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
								   <material name="white">
								   <color rgba="1 1 1 1"/>
								   </material>
								   </visual>
								   </link>
								   
								   <joint name="base_to_right_leg" type="fixed">
								   <parent link="base_link"/>
								   <child link="right_leg"/>
								   <origin xyz="0.22 0 .25"/>
								   </joint>
								   
								   <link name="left_leg">
								   <visual>
								   <geometry>
								   <box size="0.6 .2 .1"/>
								   </geometry>
								   <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
								   <material name="white"/>
								   </visual>
								   </link>
								   
								   <joint name="base_to_left_leg" type="fixed">
								   <parent link="base_link"/>
								   <child link="left_leg"/>
								   <origin xyz="-0.22 0 .25"/>
								   </joint>
								   
								   <link name="head">
								   <visual>
								   <geometry>
								   <sphere radius="0.2"/>
								   </geometry>
								   <material name="white"/>
								   </visual>
								   </link>
								   
								   <joint name="head_swivel" type="fixed">
								   <parent link="base_link"/>
								   <child link="head"/>
								   <origin xyz="0 0 0.3"/>
								   </joint>

								   <link name="box">
								   <visual>
								   <geometry>
								   <box size=".08 .08 .08"/>
								   </geometry>
								   <material name="blue"/>
								   </visual>
								   </link>
								   
								   <joint name="tobox" type="fixed">
								   <parent link="head"/>
								   <child link="box"/>
								   <origin xyz="0 0.1414 0.1414"/>
								   </joint>
								   
								   </robot>


);

tukk urdf_char_r2d2 = MSTRINGIFY(

								   <?xml version="1.0"?>
								   <robot name="visual">
								   <link name="base_link">
								   <visual>
								   <geometry>
								   <cylinder length="0.6" radius="0.2"/>
								   </geometry>
								   <material name="blue">
								   <color rgba="0 0 .8 1"/>
								   </material>
								   </visual>
								   </link>
								   
								   <link name="right_leg">
								   <visual>
								   <geometry>
								   <box size="0.6 .2 .1"/>
								   </geometry>
								   <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
								   <material name="white">
								   <color rgba="1 1 1 1"/>
								   </material>
								   </visual>
								   </link>
								   
								   <joint name="base_to_right_leg" type="fixed">
								   <parent link="base_link"/>
								   <child link="right_leg"/>
								   <origin xyz="0.22 0 .25"/>
								   </joint>
								   
								   <link name="right_base">
								   <visual>
								   <geometry>
								   <box size=".1 0.4 .1"/>
								   </geometry>
								   <material name="white"/>
								   </visual>
								   </link>
								   
								   <joint name="right_base_joint" type="fixed">
								   <parent link="right_leg"/>
								   <child link="right_base"/>
								   <origin xyz="0 0 -0.6"/>
								   </joint>
								   
								   <link name="right_front_wheel">
								   <visual>
								   <geometry>
								   <cylinder length=".1" radius="0.035"/>
								   </geometry>
								   <material name="black">
								   <color rgba="0 0 0 1"/>
								   </material>
								   </visual>
								   </link>
								   
								   <joint name="right_front_wheel_joint" type="fixed">
								   <parent link="right_base"/>
								   <child link="right_front_wheel"/>
								   <origin rpy="0 1.57075 0" xyz="0 0.133333333333 -0.085"/>
								   </joint>
								   
								   <link name="right_back_wheel">
								   <visual>
								   <geometry>
								   <cylinder length=".1" radius="0.035"/>
								   </geometry>
								   <material name="black"/>
								   </visual>
								   </link>
								   
								   <joint name="right_back_wheel_joint" type="fixed">
								   <parent link="right_base"/>
								   <child link="right_back_wheel"/>
								   <origin rpy="0 1.57075 0" xyz="0 -0.133333333333 -0.085"/>
								   </joint>
								   
								   <link name="left_leg">
								   <visual>
								   <geometry>
								   <box size="0.6 .2 .1"/>
								   </geometry>
								   <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
								   <material name="white"/>
								   </visual>
								   </link>
								   
								   <joint name="base_to_left_leg" type="fixed">
								   <parent link="base_link"/>
								   <child link="left_leg"/>
								   <origin xyz="-0.22 0 .25"/>
								   </joint>
								   
								   <link name="left_base">
								   <visual>
								   <geometry>
								   <box size=".1 0.4 .1"/>
								   </geometry>
								   <material name="white"/>
								   </visual>
								   </link>
								   
								   <joint name="left_base_joint" type="fixed">
								   <parent link="left_leg"/>
								   <child link="left_base"/>
								   <origin xyz="0 0 -0.6"/>
								   </joint>
								   
								   <link name="left_front_wheel">
								   <visual>
								   <geometry>
								   <cylinder length=".1" radius="0.035"/>
								   </geometry>
								   <material name="black"/>
								   </visual>
								   </link>
								   
								   <joint name="left_front_wheel_joint" type="fixed">
								   <parent link="left_base"/>
								   <child link="left_front_wheel"/>
								   <origin rpy="0 1.57075 0" xyz="0 0.133333333333 -0.085"/>
								   </joint>
								   
								   <link name="left_back_wheel">
								   <visual>
								   <geometry>
								   <cylinder length=".1" radius="0.035"/>
								   </geometry>
								   <material name="black"/>
								   </visual>
								   </link>
								   
								   <joint name="left_back_wheel_joint" type="fixed">
								   <parent link="left_base"/>
								   <child link="left_back_wheel"/>
								   <origin rpy="0 1.57075 0" xyz="0 -0.133333333333 -0.085"/>
								   </joint>
								   
								   <joint name="gripper_extension" type="fixed">
								   <parent link="base_link"/>
								   <child link="gripper_pole"/>
								   <origin rpy="0 0 1.57075" xyz="0 0.19 .2"/>
								   </joint>
								   
								   <link name="gripper_pole">
								   <visual>
								   <geometry>
								   <cylinder length="0.2" radius=".01"/>
								   </geometry>
								   <origin rpy="0 1.57075 0 " xyz="0.1 0 0"/>
								   <material name="Gray">
								   <color rgba=".7 .7 .7 1"/>
								   </material>
								   </visual>
								   </link>
								   
								   <joint name="left_gripper_joint" type="fixed">
								   <origin rpy="0 0 0" xyz="0.2 0.01 0"/>
								   <parent link="gripper_pole"/>
								   <child link="left_gripper"/>
								   </joint>
								   
								   <link name="left_gripper">
								   <visual>
								   <origin rpy="0.0 0 0" xyz="0 0 0"/>
								   <geometry>
								   <mesh filename="package://pr2_description/meshes/gripper_v0/l_finger.dae"/>
								   </geometry>
								   </visual>
								   </link>
								   
								   <joint name="left_tip_joint" type="fixed">
								   <parent link="left_gripper"/>
								   <child link="left_tip"/>
								   </joint>
								   
								   <link name="left_tip">
								   <visual>
								   <origin rpy="0.0 0 0" xyz="0.09137 0.00495 0"/>
								   <geometry>
								   <mesh filename="package://pr2_description/meshes/gripper_v0/l_finger_tip.dae"/>
								   </geometry>
								   </visual>
								   </link>
								   
								   <joint name="right_gripper_joint" type="fixed">
								   <origin rpy="0 0 0" xyz="0.2 -0.01 0"/>
								   <parent link="gripper_pole"/>
								   <child link="right_gripper"/>
								   </joint>
								   
								   <link name="right_gripper">
								   <visual>
								   <origin rpy="-3.1415 0 0" xyz="0 0 0"/>
								   <geometry>
								   <mesh filename="package://pr2_description/meshes/gripper_v0/l_finger.dae"/>
								   </geometry>
								   </visual>
								   </link>
								   
								   <joint name="right_tip_joint" type="fixed">
								   <parent link="right_gripper"/>
								   <child link="right_tip"/>
								   </joint>
								   
								   <link name="right_tip">
								   <visual>
								   <origin rpy="-3.1415 0 0" xyz="0.09137 0.00495 0"/>
								   <geometry>
								   <mesh filename="package://pr2_description/meshes/gripper_v0/l_finger_tip.dae"/>
								   </geometry>
								   </visual>
								   </link>
								   
								   <link name="head">
								   <visual>
								   <geometry>
								   <sphere radius="0.2"/>
								   </geometry>
								   <material name="white"/>
								   </visual>
								   </link>
								   
								   <joint name="head_swivel" type="fixed">
								   <parent link="base_link"/>
								   <child link="head"/>
								   <origin xyz="0 0 0.3"/>
								   </joint>
								   
								   <link name="box">
								   <visual>
								   <geometry>
								   <box size=".08 .08 .08"/>
								   </geometry>
								   <material name="blue"/>
								   </visual>
								   </link>
								   
								   <joint name="tobox" type="fixed">
								   <parent link="head"/>
								   <child link="box"/>
								   <origin xyz="0 0.1414 0.1414"/>
								   </joint>
								   
								   </robot>
);

tukk urdf_char = MSTRINGIFY(

								<?xml version="1.0"?>
<robot name="physics">
  <link name="base_link">
    <visual>
      <geometry>
        <cylinder length="0.6" radius="0.2"/>
      </geometry>
      <material name="blue">
        <color rgba="0 0 .8 1"/>
      </material>
    </visual>
    <collision>
      <geometry>
        <cylinder length="0.6" radius="0.2"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="10"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <link name="right_leg">
    <visual>
      <geometry>
        <box size="0.6 .2 .1"/>
      </geometry>
      <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
      <material name="white">
        <color rgba="1 1 1 1"/>
      </material>
    </visual>
    <collision>
      <geometry>
        <box size="0.6 .2 .1"/>
      </geometry>
      <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
    </collision>
    <inertial>
      <mass value="10"/>
      <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="base_to_right_leg" type="fixed">
    <parent link="base_link"/>
    <child link="right_leg"/>
    <origin xyz="0.22 0 .25"/>
  </joint>

  <link name="right_base">
    <visual>
      <geometry>
        <box size=".1 0.4 .1"/>
      </geometry>
      <material name="white"/>
    </visual>
    <collision>
      <geometry>
        <box size=".1 0.4 .1"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="10"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="right_base_joint" type="fixed">
    <parent link="right_leg"/>
    <child link="right_base"/>
    <origin xyz="0 0 -0.6"/>
  </joint>

  <link name="right_front_wheel">
    <visual>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
      <material name="black">
        <color rgba="0 0 0 1"/>
      </material>
    </visual>
    <collision>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="1"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="right_front_wheel_joint" type="continuous">
    <axis xyz="0 0 1"/>
    <parent link="right_base"/>
    <child link="right_front_wheel"/>
    <origin rpy="0 1.57075 0" xyz="0 0.133333333333 -0.085"/>
    <limit effort="100" velocity="100"/>
    <joint_properties damping="0.0" friction="0.0"/>
  </joint>

  <link name="right_back_wheel">
    <visual>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
      <material name="black"/>
    </visual>
    <collision>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="1"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="right_back_wheel_joint" type="continuous">
    <axis xyz="0 0 1"/>
    <parent link="right_base"/>
    <child link="right_back_wheel"/>
    <origin rpy="0 1.57075 0" xyz="0 -0.133333333333 -0.085"/>
    <limit effort="100" velocity="100"/>
    <joint_properties damping="0.0" friction="0.0"/>
  </joint>

  <link name="left_leg">
    <visual>
      <geometry>
        <box size="0.6 .2 .1"/>
      </geometry>
      <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
      <material name="white"/>
    </visual>
    <collision>
      <geometry>
        <box size="0.6 .2 .1"/>
      </geometry>
      <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
    </collision>
    <inertial>
      <mass value="10"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
       <origin rpy="0 1.57075 0" xyz="0 0 -0.3"/>
    </inertial>
  </link>

  <joint name="base_to_left_leg" type="fixed">
    <parent link="base_link"/>
    <child link="left_leg"/>
    <origin xyz="-0.22 0 .25"/>
  </joint>

  <link name="left_base">
    <visual>
      <geometry>
        <box size=".1 0.4 .1"/>
      </geometry>
      <material name="white"/>
    </visual>
    <collision>
      <geometry>
        <box size=".1 0.4 .1"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="10"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="left_base_joint" type="fixed">
    <parent link="left_leg"/>
    <child link="left_base"/>
    <origin xyz="0 0 -0.6"/>
  </joint>

  <link name="left_front_wheel">
    <visual>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
      <material name="black"/>
    </visual>
    <collision>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="1"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="left_front_wheel_joint" type="continuous">
    <axis xyz="0 0 1"/>
    <parent link="left_base"/>
    <child link="left_front_wheel"/>
    <origin rpy="0 1.57075 0" xyz="0 0.133333333333 -0.085"/>
    <limit effort="100" velocity="100"/>
    <joint_properties damping="0.0" friction="0.0"/>
  </joint>

  <link name="left_back_wheel">
    <visual>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
      <material name="black"/>
    </visual>
    <collision>
      <geometry>
        <cylinder length=".1" radius="0.035"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="1"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="left_back_wheel_joint" type="continuous">
    <axis xyz="0 0 1"/>
    <parent link="left_base"/>
    <child link="left_back_wheel"/>
    <origin rpy="0 1.57075 0" xyz="0 -0.133333333333 -0.085"/>
    <limit effort="100" velocity="100"/>
    <joint_properties damping="0.0" friction="0.0"/>
  </joint>
  <joint name="gripper_extension" type="prismatic">
    <parent link="base_link"/>
    <child link="gripper_pole"/>
    <limit effort="1000.0" lower="-0.38" upper="0" velocity="0.5"/>
    <origin rpy="0 0 1.57075" xyz="0 0.19 .2"/>
  </joint>

  <link name="gripper_pole">
    <visual>
      <geometry>
        <cylinder length="0.2" radius=".01"/>
      </geometry>
      <origin rpy="0 1.57075 0 " xyz="0.1 0 0"/>
      <material name="Gray">
        <color rgba=".7 .7 .7 1"/>
      </material>
    </visual>
    <collision>
      <geometry>
        <cylinder length="0.2" radius=".01"/>
      </geometry>
      <origin rpy="0 1.57075 0 " xyz="0.1 0 0"/>
    </collision>
    <inertial>
      <mass value="0.05"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="left_gripper_joint" type="revolute">
    <axis xyz="0 0 1"/>
    <limit effort="1000.0" lower="0.0" upper="0.548" velocity="0.5"/>
    <origin rpy="0 0 0" xyz="0.2 0.01 0"/>
    <parent link="gripper_pole"/>
    <child link="left_gripper"/>
  </joint>

  <link name="left_gripper">
    <visual>
      <origin rpy="0.0 0 0" xyz="0 0 0"/>
      <geometry>
        <mesh filename="l_finger.stl"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <mesh filename="l_finger.stl"/>
      </geometry>
      <origin rpy="0.0 0 0" xyz="0 0 0"/>
    </collision>
    <inertial>
      <mass value="0.05"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="left_tip_joint" type="fixed">
    <parent link="left_gripper"/>
    <child link="left_tip"/>
  </joint>

  <link name="left_tip">
    <visual>
      <origin rpy="0.0 0 0" xyz="0.09137 0.00495 0"/>
      <geometry>
        <mesh filename="l_finger_tip.stl"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <mesh filename="l_finger_tip.stl"/>
      </geometry>
      <origin rpy="0.0 0 0" xyz="0.09137 0.00495 0"/>
    </collision>
    <inertial>
      <mass value="0.05"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="right_gripper_joint" type="revolute">
    <axis xyz="0 0 -1"/>
    <limit effort="1000.0" lower="0.0" upper="0.548" velocity="0.5"/>
    <origin rpy="0 0 0" xyz="0.2 -0.01 0"/>
    <parent link="gripper_pole"/>
    <child link="right_gripper"/>
  </joint>

  <link name="right_gripper">
    <visual>
      <origin rpy="-3.1415 0 0" xyz="0 0 0"/>
      <geometry>
        <mesh filename="l_finger.stl"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <mesh filename="l_finger.stl"/>
      </geometry>
      <origin rpy="-3.1415 0 0" xyz="0 0 0"/>
    </collision>
    <inertial>
      <mass value="0.05"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="right_tip_joint" type="fixed">
    <parent link="right_gripper"/>
    <child link="right_tip"/>
  </joint>

  <link name="right_tip">
    <visual>
      <origin rpy="-3.1415 0 0" xyz="0.09137 0.00495 0"/>
      <geometry>
        <mesh filename="l_finger_tip.stl"/>
      </geometry>
    </visual>
    <collision>
      <geometry>
        <mesh filename="l_finger_tip.stl"/>
      </geometry>
      <origin rpy="-3.1415 0 0" xyz="0.09137 0.00495 0"/>
    </collision>
    <inertial>
      <mass value="0.05"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <link name="head">
    <visual>
      <geometry>
        <sphere radius="0.2"/>
      </geometry>
      <material name="white"/>
    </visual>
    <collision>
      <geometry>
        <sphere radius="0.2"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="10"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="head_swivel" type="continuous">
    <parent link="base_link"/>
    <child link="head"/>
    <axis xyz="0 0 1"/>
    <origin xyz="0 0 0.3"/>
  </joint>

  <link name="box">
    <visual>
      <geometry>
        <box size=".08 .08 .08"/>
      </geometry>
      <material name="blue"/>
    </visual>
    <collision>
      <geometry>
        <box size=".08 .08 .08"/>
      </geometry>
    </collision>
    <inertial>
      <mass value="1"/>
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="1.0" iyz="0.0" izz="1.0"/>
    </inertial>
  </link>

  <joint name="tobox" type="fixed">
    <parent link="head"/>
    <child link="box"/>
    <origin xyz="0 0.1414 0.1414"/>
  </joint>

</robot>


);

#endif  //URDF_SAMPLES_H
