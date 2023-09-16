using System;
using System.Runtime.CompilerServices;
using System.Runtime.ConstrainedExecution;
using System.Collections.Generic;
namespace KaidelCore
{

	public class Entity
	{
		private static Dictionary<Entity, Dictionary<Type,Component>> s_Storage = new Dictionary<Entity, Dictionary<Type, Component>>();

		internal readonly ulong ID;
		internal Entity(ulong id)
		{
			ID = id;
		}
		protected Entity() 
		{ 
			ID = 0;
		}
		public Vector3 Position { 
			get
			{
				Internals.TransformComponent_GetPosition(ID, out Vector3 position);
				return position;
			}
			set
			{
				Internals.TransformComponent_SetPosition(ID,ref value);
			}
		}
		public Vector3 Rotation
		{
			get
			{
				Internals.TransformComponent_GetPosition(ID, out Vector3 rotation);
				return rotation;
			}
			set
			{
				Internals.TransformComponent_SetRotation(ID,ref value);
			}
		}
		public Vector3 Scale
		{
			get
			{
				Internals.TransformComponent_GetPosition(ID, out Vector3 scale);
				return scale;
			}
			set
			{
				Internals.TransformComponent_SetScale(ID, ref value);
			}
		}
		public string Name
		{
			get
			{
				return Internals.Entity_GetName(ID);
			}
		}

		public virtual void OnCreate() { }
		public virtual void OnUpdate(float ts) { }
		public bool HasComponent<T>() where T : Component ,new(){ 
			Type componentType = typeof(T);
			return Internals.Entity_HasComponent(ID, componentType);
		} 
		public T GetComponent<T>() where T : Component ,new(){
			if (!HasComponent<T>())
				return null;
			return new T() { Entity = this };
		}
		public T AddComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			Internals.Entity_AddComponent(ID, componentType);
			return new T() { Entity = this };
		}
	

	}
}
