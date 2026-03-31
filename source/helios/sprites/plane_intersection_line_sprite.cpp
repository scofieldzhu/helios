/*******************************************************
 * All Copyright (C) by Sysbot Co. ltd (2025-2025)
 * author: zhucg
 * time:2025/6/24
 *******************************************************/
#include "plane_intersection_line_sprite.h"
#include <vtkTransform.h>
#include <vtkLine.h>
#include "mirfak/core/scene.h"
#include "mirfak/core/slice_plane_sprite.h"
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

namespace{
    constexpr double kOffsetToleranceOnLine = 10.0;
    constexpr double kDistToleranceToEndLinePoint = 10.0;
    constexpr double kDistToleranceToLineCenterPoint = 10.0;

    using DoubleOpt = std::optional<double>;
    using Pt2Opt = std::optional<Point2>;
    using Pt2Vec = std::vector<Point2>;

    struct LinearEquation
    {
        Pt2Opt intersectWithLine(const LinearEquation& other_line)const{
            double A1 = A;
            double B1 = B;
            double C1 = C;
            double A2 = other_line.A;
            double B2 = other_line.B;
            double C2 = other_line.C;
            double det = A1 * B2 - A2 * B1;
            if(fabs(det) < 1e-10){
                // parallel or coincidence
                if(fabs(A1 * C2 - A2 * C1) < 1e-10) { 
                    // coincidence, return infinite results
                    return std::nullopt;
                }else{
                    // parallel, no intersection point
                    return std::nullopt;
                }
            }
            // only one intersection
            double x = (B1 * C2 - B2 * C1) / det;
            double y = (A2 * C1 - A1 * C2) / det;
            return Point2{x, y};
        }

        bool isInViewport(const Arry4d& viewport, const Point2& pt)const{
            double min_x = viewport[0];
            double max_x = viewport[2];
            double min_y = viewport[1];
            double max_y = viewport[3];
            return pt.x >= min_x && max_x >= pt.x && pt.y >= min_y && max_y >= pt.y;
        }

        Pt2Vec intersectWithViewport(const Arry4d& viewport)const{
            Pt2Vec result_pts;
            if(isInViewport(viewport, p1) && isInViewport(viewport, p2)){
                result_pts.push_back(p1);
                result_pts.push_back(p2);
                return result_pts;
            }
            double min_x = viewport[0];
            double max_x = viewport[2];
            double min_y = viewport[1];
            double max_y = viewport[3];
            LinearEquation left(min_x, min_y, min_x, max_y);
            LinearEquation right(max_x, min_y, max_x, max_y);
            LinearEquation top(min_x, max_y, max_x, max_y);
            LinearEquation bottom(min_x, min_y, max_x, min_y);            
            auto pt_opt = intersectWithLine(left);
            if(pt_opt){
                if((*pt_opt).y > min_y && (*pt_opt).y < max_y){
                    result_pts.push_back(*pt_opt);
                }
            }
            pt_opt = intersectWithLine(right);
            if(pt_opt){
                if((*pt_opt).y > min_y && (*pt_opt).y < max_y){
                    result_pts.push_back(*pt_opt);
                }
            }
            pt_opt = intersectWithLine(top);
            if(pt_opt){
                if((*pt_opt).x > min_x && (*pt_opt).x < max_x){
                    result_pts.push_back(*pt_opt);
                }
            }
            pt_opt = intersectWithLine(bottom);
            if(pt_opt){
                if((*pt_opt).x > min_x && (*pt_opt).x < max_x){
                    result_pts.push_back(*pt_opt);
                }
            }
            if(result_pts.size() == 2){
                //move point closet to p1 forward!
                if(result_pts[0].distanceTo(p1) > result_pts[1].distanceTo(p1)){
                    //swap two values
                    auto tmp = result_pts[0];
                    result_pts[0] = result_pts[1];
                    result_pts[1] = tmp;
                }
                if(isInViewport(viewport, p1)){
                    result_pts[0] = p1;
                }else if(isInViewport(viewport, p2)){
                    result_pts[1] = p2;
                }
            }
            return result_pts;
        }

        LinearEquation() = default;
        LinearEquation(double x1, double y1, double x2, double y2)
            :p1(x1, y1),
            p2(x2, y2){
            A = y1 - y2;
            B = x2 - x1;
            C = x1 * y2 - x2 * y1;
        }
        Point2 p1;
        Point2 p2;
        double A = 0.0;
        double B = 0.0;
        double C = 0.0;
    };
}

PlaneIntersectionLineSprite::PlaneIntersectionLineSprite()
    :intersection_line_(std::make_unique<PlaneCutterSprite>()),
    cross_intersection_line_(std::make_unique<PlaneCutterSprite>())
{
    addChild(*intersection_line_);
    addChild(*cross_intersection_line_);
}

PlaneIntersectionLineSprite::~PlaneIntersectionLineSprite()
{
}

void PlaneIntersectionLineSprite::setLineColor(const Color& hline_clr, const Color& vline_clr)
{
    intersection_line_->setColor(hline_clr);
    cross_intersection_line_->setColor(vline_clr);
}

void PlaneIntersectionLineSprite::setPlanes(SlicePlaneSprite &cut_plane, SlicePlaneSprite &target_plane, SlicePlaneSprite &cross_plane)
{
    cut_plane_ = &cut_plane;
    target_plane_ = &target_plane;  
    cross_plane_ = &cross_plane;
    intersection_line_->setCutPlane(cut_plane_->getPlaneEquation());
    intersection_line_->setTargetPlaneSource(target_plane_->getPlaneSource());    
    cross_intersection_line_->setCutPlane(cross_plane_->getPlaneEquation());
    cross_intersection_line_->setTargetPlaneSource(target_plane_->getPlaneSource());    
}

int PlaneIntersectionLineSprite::computeEditState(Scene& s, int x, int y, int modify)
{
    auto world_to_display_point_func = [&s](const Point3& wpt){
        auto dis_pt = s.worldToDisplay(wpt);
        return Point3(dis_pt.x(), dis_pt.y(), 0.0);
    };

    auto view_geo = s.getDisplayGeometry();
    double min_x = static_cast<double>(view_geo[0]);
    double max_x = static_cast<double>(view_geo[0] + view_geo[2]);
    double min_y = static_cast<double>(view_geo[1]);
    double max_y = static_cast<double>(view_geo[1] + view_geo[3]);
    Arry4d display_viewport = std::to_array({min_x, min_y, max_x, max_y});

    Point3 d_pos((double)x, (double)y, 0.0);
    state_data_.scene = &s;
    state_data_.event_dpos = {x, y};
    state_data_.picked_plane = nullptr;
    state_data_.on_plane = nullptr;
    state_data_.cross_plane = nullptr;
    edit_state_ = IS_OUTSIDE;

    //calculate some convenient data on horizontal line
    auto hline_points = intersection_line_->getLinePoints();    
    if(!hline_points.has_value()){
        return edit_state_;
    }
    Point3 line_p1 = world_to_display_point_func((*hline_points)[0]);
    Point3 line_p2 = world_to_display_point_func((*hline_points)[1]);
    LinearEquation hline_equation(line_p1.x(), line_p1.y(), line_p2.x(), line_p2.y());
    auto interection_points = hline_equation.intersectWithViewport(display_viewport);
    if(interection_points.empty()){
        SPDLOG_DEBUG("No intersection!");
        return edit_state_;
    }
    line_p1 = {interection_points[0].x, interection_points[0].y, 0.0};
    line_p2 = {interection_points[1].x, interection_points[1].y, 0.0};
    auto dist_to_hline = std::sqrt(vtkLine::DistanceToLine(d_pos, line_p1, line_p2));
    Point3 closest_h_end_pt = (d_pos.distanceTo(line_p1) < d_pos.distanceTo(line_p2) ? line_p1 : line_p2);
    auto dist_to_hline_end = d_pos.distanceTo(closest_h_end_pt);

    bool projection_is_on_hline = false;
    auto hline_segment = Line::FromTwoPoints(line_p1, line_p2);
    auto proj_pt = hline_segment.projectPoint(d_pos);
    auto line_length = line_p1.distanceTo(line_p2);
    projection_is_on_hline = (proj_pt.distanceTo(line_p1) < line_length && proj_pt.distanceTo(line_p2) < line_length);
    //SPDLOG_DEBUG(">>intersection points: pt1:{} pt2:{} dist_to_hline:{} dist_to_hline_end:{}", line_p1.toStr(), line_p2.toStr(), dist_to_hline, dist_to_hline_end);

    //calculate some convenient data on vertical line
    auto vline_points = cross_intersection_line_->getLinePoints();    
    if(!vline_points.has_value()){
        return edit_state_;
    }
    line_p1 = world_to_display_point_func((*vline_points)[0]); 
    line_p2 = world_to_display_point_func((*vline_points)[1]); 
    LinearEquation vline_equation(line_p1.x(), line_p1.y(), line_p2.x(), line_p2.y());
    interection_points = vline_equation.intersectWithViewport(display_viewport);
    if(interection_points.empty()){
        SPDLOG_DEBUG("No intersection!");
        return edit_state_;
    }
    line_p1 = {interection_points[0].x, interection_points[0].y, 0.0};
    line_p2 = {interection_points[1].x, interection_points[1].y, 0.0};
    auto dist_to_vline = std::sqrt(vtkLine::DistanceToLine(d_pos, line_p1, line_p2));
    Point3 closest_v_end_pt = (d_pos.distanceTo(line_p1) < d_pos.distanceTo(line_p2) ? line_p1 : line_p2);
    auto dist_to_vline_end = d_pos.distanceTo(closest_v_end_pt);
    auto calc_ortho_center = [this](){
        Plane p1(target_plane_->getOrigin(), target_plane_->getNormal());
        Plane p2(cut_plane_->getOrigin(), cut_plane_->getNormal());
        Plane p3(cross_plane_->getOrigin(), cross_plane_->getNormal());
        return Plane::CalcCenterOfTriplePlanes(p1, p2, p3).value();
    };
    auto w_ortho_center = calc_ortho_center();
    auto d_ortho_center_pt = world_to_display_point_func(w_ortho_center); 
    auto dist_to_ortho_center = d_pos.distanceTo(d_ortho_center_pt);

    bool projection_is_on_vline = false;
    auto vline_segment = Line::FromTwoPoints(line_p1, line_p2);
    proj_pt = vline_segment.projectPoint(d_pos);
    line_length = line_p1.distanceTo(line_p2);
    projection_is_on_vline = (proj_pt.distanceTo(line_p1) < line_length && proj_pt.distanceTo(line_p2) < line_length);
    //SPDLOG_DEBUG(">>intersection points: pt1:{} pt2:{} dist_to_vline:{} dist_to_vline_end:{} dist_to_v_center:{}", line_p1.toStr(), line_p2.toStr(), dist_to_vline, dist_to_vline_end, dist_to_ortho_center);

    //make judgement for different states
    if(dist_to_vline_end <= kDistToleranceToEndLinePoint || dist_to_hline_end <= kDistToleranceToEndLinePoint){
        edit_state_ = IS_ROTATING;
        state_data_.on_plane = target_plane_;
        state_data_.picked_plane = (dist_to_vline_end <= kDistToleranceToEndLinePoint) ? cut_plane_ : cross_plane_;
        state_data_.cross_plane = (dist_to_vline_end <= kDistToleranceToEndLinePoint) ? cross_plane_ : cut_plane_;
        return edit_state_;
    }
    if(dist_to_ortho_center <= kDistToleranceToLineCenterPoint){
        edit_state_ = IS_MOVE_CENTER;
        state_data_.on_plane = target_plane_;
        state_data_.picked_plane = cut_plane_;
        state_data_.cross_plane = cross_plane_;
        return edit_state_;
    }        
    if(dist_to_hline <= kOffsetToleranceOnLine && projection_is_on_hline){
        //calculate angle between pos line and horizontal line
        Point2 d_pt1 = {d_pos.x(), d_ortho_center_pt.y()};
        Vec2 d_pt0 = {d_ortho_center_pt[0], d_ortho_center_pt[1]};
        Vec2 d_vec1 = d_pt1 - d_pt0;
        Vec2 d_pt2 = {d_pos[0], d_pos[1]};
        Vec2 d_vec2 = d_pt2 - d_pt0;
        auto z = d_vec1.x * d_vec2.y - d_vec1.y * d_vec2.x;
        auto dot = d_vec1.dot(d_vec2);
        double vec_angle = RadianToDegree(atan2(z, dot));
        //SPDLOG_DEBUG("vec_angle:{}", vec_angle);
        edit_state_ = IS_TRANSLATE_H;
        state_data_.angle = vec_angle;
        state_data_.on_plane = target_plane_;
        state_data_.picked_plane = cut_plane_;
        state_data_.cross_plane = cross_plane_;
        return edit_state_;
    }
    if(dist_to_vline <= kOffsetToleranceOnLine && projection_is_on_vline){
        //calculate angle between pos line and vertical line
        Point2 d_pt1 = {d_ortho_center_pt.x(), d_pos.y()};
        Vec2 d_pt0 = {d_ortho_center_pt[0], d_ortho_center_pt[1]};
        Vec2 d_vec1 = d_pt1 - d_pt0;
        Vec2 d_pt2 = {d_pos[0], d_pos[1]};
        Vec2 d_vec2 = d_pt2 - d_pt0;
        auto z = d_vec1.x * d_vec2.y - d_vec1.y * d_vec2.x;
        auto dot = d_vec1.dot(d_vec2);
        double vec_angle = RadianToDegree(atan2(z, dot));
        //SPDLOG_DEBUG("vec_angle:{}", vec_angle);
        edit_state_ = IS_TRANSLATE_V;
        state_data_.angle = vec_angle;
        state_data_.on_plane = target_plane_;
        state_data_.picked_plane = cross_plane_;
        state_data_.cross_plane = cut_plane_;
        return edit_state_;
    }
    //none of intersection lines picked!
    edit_state_ = IS_OUTSIDE;
    state_data_.on_plane = target_plane_;
    return edit_state_;
}

const void* PlaneIntersectionLineSprite::getEditStateData()const
{
    return reinterpret_cast<const void*>(&state_data_);
}

NAMESPACE_END