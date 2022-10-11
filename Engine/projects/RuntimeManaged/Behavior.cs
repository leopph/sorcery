using System;
using System.Runtime.CompilerServices;

namespace leopph
{
    public class Behavior
    {
        public Entity Entity
        {
            get
            {
                if (_entity == null)
                {
                    _entity = new Entity(InternalGetEntityId(_id));
                }
                return _entity;

            }
        }

        private ulong _id;
        private Entity? _entity;

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong InternalGetEntityId(ulong behaviorId);
    }
}
