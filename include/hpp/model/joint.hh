///
/// Copyright (c) 2013, 2014 CNRS
/// Author: Florent Lamiraux
///
///
// This file is part of hpp-model
// hpp-model is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-model is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-model  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef HPP_MODEL_JOINT_HH
# define HPP_MODEL_JOINT_HH

# include <cstddef>
# include <fcl/math/transform.h>
# include <hpp/model/config.hh>
# include <hpp/model/fwd.hh>

namespace hpp {
  namespace model {
    /// Robot joint
    ///
    /// A joint maps an input vector to a transformation of SE(3) from the
    /// parent frame to the joint frame.
    /// 
    /// The input vector is provided through the configuration vector of the
    /// robot the joint belongs to. The joint input vector is composed of the
    /// components of the robot configuration starting at
    /// Joint::rankInConfiguration.
    ///
    /// The joint input vector represents a element of a Lie group, either
    /// \li a vector space for JointTranslation, and bounded JointRotation,
    /// \li the unit circle for non-bounded JointRotation joints,
    /// \li an element of SO(3) for JointSO3, represented by a unit quaternion.
    ///
    /// Operations specific to joints (uniform sampling of input space, straight
    /// interpolation, distance, ...) are performed by a JointConfiguration
    /// instance that has the same class hierarchy as Joint.
    class HPP_MODEL_DLLAPI Joint {
    public:
      Joint (const Transform3f& initialPosition, std::size_t configSize,
	     std::size_t numberDof);
      virtual ~Joint ();
      /// \name Name
      /// @{
      /// Set name
      virtual inline void name(const std::string& name)
      {
	name_ = name;
      }
      /// Get name
      virtual inline const std::string& name() const
      {
	return name_;
      }
      /// @}

      /// \name Position
      /// @{
      /// Joint initial position (when robot is in zero configuration)
      const Transform3f& initialPosition () const;
      /// Joint transformation
      const Transform3f& currentTransformation () const;
      ///@}
      /// Return number of degrees of freedom
      const std::size_t& numberDof () const
      {
	return numberDof_;
      }
      /// Return number of degrees of freedom
      const std::size_t& configSize () const
      {
	return configSize_;
      }
      /// Return rank of the joint in the configuration vector
      const std::size_t& rankInConfiguration () const
      {
	return rankInConfiguration_;
      }
      /// Return rank of the joint in the velocity vector
      std::size_t rankInVelocity () const
      {
	return rankInVelocity_;
      }

      /// \name Kinematic chain
      /// @{
      /// Get a pointer to the parent joint (if any).
      JointPtr_t parentJoint () const
      {
	return parent_;
      }
      /// Add child joint
      void addChildJoint (JointPtr_t joint);

      /// Number of child joints
      std::size_t numberChildJoints () const
      {
	return children_.size ();
      }
      /// Get child joint
      JointPtr_t childJoint (std::size_t rank) const
      {
	return children_ [rank];
      }

      /// Get the rank of this joint in the robot configuration vector.
      ///
      ///@}

      /// \name Bounds
      /// @{
      /// Set whether given degree of freedom is bounded
      void isBounded (std::size_t rank, bool bounded);
      /// Get whether given degree of freedom is bounded
      bool isBounded (std::size_t rank) const;
      /// Get lower bound of given degree of freedom
      double lowerBound (std::size_t rank) const;
      /// Get upper bound of given degree of freedom
      double upperBound (std::size_t rank) const;
      /// Set lower bound of given degree of freedom
      void lowerBound (std::size_t rank, double lowerBound);
      /// Set upper bound of given degree of freedom
      void upperBound (std::size_t rank, double upperBound);
      /// @}

      /// \name Jacobian
      /// \{

      /// Get const reference to Jacobian
      const JointJacobian_t& jacobian () const
      {
	return jacobian_;
      }
      /// Get non const reference to Jacobian
      JointJacobian_t& jacobian ()
      {
	return jacobian_;
      }
      /// \}
      /// Access to configuration space
      JointConfiguration* configuration () const {return configuration_;}
      /// Set robot owning the kinematic chain
      void setRobot (Device* device) {robot_ = device;}
      /// Access robot owning the object
      Device* getRobot () { return robot_;}
      /// \name Body linked to the joint
      /// @{
      /// Get linked body
      Body* linkedBody () const;
      /// Get linked body
      void setLinkedBody (Body* body);
      /// @}

      /// Display joint
      virtual std::ostream& display (std::ostream& os) const;
    protected:
      JointConfiguration* configuration_;
      Transform3f currentTransformation_;
      Transform3f positionInParentFrame_;
      Transform3f T3f_;
      /// Mass of this and all descendants
      double mass_;
      /// Mass time center of mass of this and all descendants
      fcl::Vec3f massCom_;
   private:
      void computePosition (const Configuration_t& configuration,
			    const Transform3f& parentConfig);

      virtual void computeMotion (const Configuration_t& configuration,
				  const Transform3f& parentConfig) = 0;

      /// Write a block of Jacobian
      ///
      /// child: joint the motion of which is generated by the degrees of
      ///      freedom of this
      ///
      /// child should be a descendant of this.
      /// This method writes in the jacobian of child the motion generated by
      /// this at the current position of child. If index is the rank of this
      /// in the velocity vector, the method fills colums from index to index +
      /// this joint number of degrees of freedom.
      virtual void writeSubJacobian (const JointPtr_t& child) = 0;
      void computeJacobian ();
      /// Compute mass of this and all descendants
      double computeMass ();
      /// Compute the product m * com
      ///
      /// \li m is the mass of the joint and all descendants,
      /// \li com is the center of mass of the joint and all descendants.
      void computeMassTimesCenterOfMass ();
      virtual void writeComSubjacobian (ComJacobian_t& jacobian,
					const double& totalMass) = 0;
      std::size_t configSize_;
      std::size_t numberDof_;
      Transform3f initialPosition_;
      Body* body_;
      Device* robot_;
      std::string name_;
      std::vector <JointPtr_t> children_;
      JointPtr_t parent_;
      std::size_t rankInConfiguration_;
      std::size_t rankInVelocity_;
      JointJacobian_t jacobian_;
      /// Rank of the joint in vector of children of parent joint.
      std::size_t rankInParent_;
      friend class Device;
      friend class ChildrenIterator;
    }; // class Joint

    /// Anchor Joint
    ///
    /// An anchor joint has no degree of freedom. It is usually used as an
    /// intermediate frame in a kinematic chain, or as a root joint for a
    /// multi-robot kinematic chain.
    class HPP_MODEL_DLLAPI JointAnchor : public Joint
    {
    public:
      JointAnchor (const Transform3f& initialPosition);
      virtual ~JointAnchor ();
      virtual void computeMotion (const Configuration_t& configuration,
				    const Transform3f& parentConfig);
    private:
      virtual void writeSubJacobian (const JointPtr_t& child);
      virtual void writeComSubjacobian (ComJacobian_t& jacobian,
					const double& totalMass);
    }; // class JointAnchor

    /// Spherical Joint
    ///
    /// map a unit quaternion as input vector to a rotation of SO(3).
    class HPP_MODEL_DLLAPI JointSO3 : public Joint
    {
    public:
      JointSO3 (const Transform3f& initialPosition);
      virtual void computeMotion (const Configuration_t& configuration,
				    const Transform3f& parentConfig);
      virtual ~JointSO3 ();
    private:
      virtual void writeSubJacobian (const JointPtr_t& child);
      virtual void writeComSubjacobian (ComJacobian_t& jacobian,
					const double& totalMass);
      mutable fcl::Vec3f com_;
    }; // class JointSO3

    /// Rotation Joint
    ///
    /// Map an angle as one-dimensional input vector to a rotation around a
    /// fixed axis in parent frame.
    class HPP_MODEL_DLLAPI JointRotation : public Joint
    {
    public:
      JointRotation (const Transform3f& initialPosition);
      virtual void computeMotion (const Configuration_t& configuration,
				    const Transform3f& parentConfig);
      virtual ~JointRotation ();
    private:
      virtual void writeSubJacobian (const JointPtr_t& child);
      virtual void writeComSubjacobian (ComJacobian_t& jacobian,
					const double& totalMass);
      fcl::Matrix3f R_;
      mutable double angle_;
      mutable fcl::Vec3f axis_;
      mutable fcl::Vec3f O2O1_;
      mutable fcl::Vec3f cross_;
      mutable fcl::Vec3f com_;
    }; // class JointRotation

    /// Translation Joint
    ///
    /// Map a length as one-dimensional input vector to a translation along a
    /// fixed axis in parent frame.
    class HPP_MODEL_DLLAPI JointTranslation : public Joint
    {
    public:
      JointTranslation (const Transform3f& initialPosition);
      virtual void computeMotion (const Configuration_t& configuration,
				    const Transform3f& parentConfig);
      virtual ~JointTranslation ();
    private:
      virtual void writeSubJacobian (const JointPtr_t& child);
      virtual void writeComSubjacobian (ComJacobian_t& jacobian,
					const double& totalMass);
      fcl::Vec3f t_;
      mutable fcl::Vec3f axis_;
      mutable fcl::Vec3f com_;
    }; // class JointTranslation

  } // namespace model
} // namespace hpp

std::ostream& operator<< (std::ostream& os , const fcl::Transform3f& trans);
std::ostream& operator<< (std::ostream& os, const hpp::model::Joint& joint);

#endif // HPP_MODEL_JOINT_HH
