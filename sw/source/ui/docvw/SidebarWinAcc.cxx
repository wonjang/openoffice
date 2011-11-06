/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/




#include "precompiled_sw.hxx"

#include <SidebarWinAcc.hxx>

#include <SidebarWin.hxx>
#include <viewsh.hxx>
#include <accmap.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

namespace css = ::com::sun::star;

namespace sw { namespace sidebarwindows {

// =============================================================================
// declaration and implementation of accessible context for <SidebarWinAccessible> instance
// =============================================================================
class SidebarWinAccessibleContext : public VCLXAccessibleComponent
{
    public:
        explicit SidebarWinAccessibleContext( SwSidebarWin& rSidebarWin,
                                              ViewShell& rViewShell,
                                              const SwFrm* pAnchorFrm )
            : VCLXAccessibleComponent( rSidebarWin.GetWindowPeer() )
            , mrViewShell( rViewShell )
            , mpAnchorFrm( pAnchorFrm )
            , maMutex()
        {
            rSidebarWin.SetAccessibleRole( css::accessibility::AccessibleRole::COMMENT );
        }

        virtual ~SidebarWinAccessibleContext()
        {}

        void ChangeAnchor( const SwFrm* pAnchorFrm )
        {
            vos::OGuard aGuard(maMutex);

            mpAnchorFrm = pAnchorFrm;
        }

        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
            getAccessibleParent() throw (css::uno::RuntimeException)
        {
            vos::OGuard aGuard(maMutex);

            css::uno::Reference< css::accessibility::XAccessible > xAccParent;

            if ( mpAnchorFrm &&
                 mrViewShell.GetAccessibleMap() )
            {
                xAccParent = mrViewShell.GetAccessibleMap()->GetContext( mpAnchorFrm, sal_False );
            }

            return xAccParent;
        }

        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException)
        {
            vos::OGuard aGuard(maMutex);

            sal_Int32 nIndex( -1 );

            if ( mpAnchorFrm && GetWindow() &&
                 mrViewShell.GetAccessibleMap() )
            {
                nIndex = mrViewShell.GetAccessibleMap()->GetChildIndex( *mpAnchorFrm,
                                                                        *GetWindow() );
            }

            return nIndex;
        }

    private:
        ViewShell& mrViewShell;
        const SwFrm* mpAnchorFrm;

        ::vos::OMutex maMutex;
};

// =============================================================================
// implementaion of accessible for <SwSidebarWin> instance
// =============================================================================
SidebarWinAccessible::SidebarWinAccessible( SwSidebarWin& rSidebarWin,
                                            ViewShell& rViewShell,
                                            const SwSidebarItem& rSidebarItem )
    : VCLXWindow()
    , mrSidebarWin( rSidebarWin )
    , mrViewShell( rViewShell )
    , mpAnchorFrm( rSidebarItem.maLayoutInfo.mpAnchorFrm )
    , bAccContextCreated( false )
{
    SetWindow( &mrSidebarWin );
}

SidebarWinAccessible::~SidebarWinAccessible()
{
}

void SidebarWinAccessible::ChangeSidebarItem( const SwSidebarItem& rSidebarItem )
{
    if ( bAccContextCreated )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext > xAcc
                                                    = getAccessibleContext();
        if ( xAcc.is() )
        {
            SidebarWinAccessibleContext* pAccContext =
                        dynamic_cast<SidebarWinAccessibleContext*>(xAcc.get());
            if ( pAccContext )
            {
                pAccContext->ChangeAnchor( rSidebarItem.maLayoutInfo.mpAnchorFrm );
            }
        }
    }
}

css::uno::Reference< css::accessibility::XAccessibleContext > SidebarWinAccessible::CreateAccessibleContext()
{
    SidebarWinAccessibleContext* pAccContext =
                                new SidebarWinAccessibleContext( mrSidebarWin,
                                                                 mrViewShell,
                                                                 mpAnchorFrm );
    css::uno::Reference< css::accessibility::XAccessibleContext > xAcc( pAccContext );
    bAccContextCreated = true;
    return xAcc;
}

} } // end of namespace sw::sidebarwindows

